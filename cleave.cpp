#include "cleave.hpp"
#include <QDateTime>
#include <QProcess>
#include <QtMath>
#include <algorithm>
#include <cstring>

static void fftInPlace(float *re, float *im, int n) {
  for (int i = 1, j = 0; i < n; ++i) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1)
      j ^= bit;
    j ^= bit;
    if (i < j) {
      std::swap(re[i], re[j]);
      std::swap(im[i], im[j]);
    }
  }
  for (int len = 2; len <= n; len <<= 1) {
    float ang = -2.0f * float(M_PI) / len;
    float wRe = cosf(ang), wIm = sinf(ang);
    for (int i = 0; i < n; i += len) {
      float cRe = 1.0f, cIm = 0.0f;
      for (int j = 0; j < len / 2; ++j) {
        float uRe = re[i + j], uIm = im[i + j];
        float vRe = re[i + j + len / 2], vIm = im[i + j + len / 2];
        float tRe = cRe * vRe - cIm * vIm, tIm = cRe * vIm + cIm * vRe;
        re[i + j] = uRe + tRe;
        im[i + j] = uIm + tIm;
        re[i + j + len / 2] = uRe - tRe;
        im[i + j + len / 2] = uIm - tIm;
        float nr = cRe * wRe - cIm * wIm;
        cIm = cRe * wIm + cIm * wRe;
        cRe = nr;
      }
    }
  }
}

Cleave::Cleave(QObject *parent) : QObject(parent) {
  m_mag.resize(m_bandCount, 0.0f);
  m_pk.resize(m_bandCount, 0.0f);
  m_pcmBuf.reserve(kBufCap);
  for (int i = 0; i < kFFTSize; ++i)
    m_win[i] = 0.5f * (1.0f - cosf(2.0f * float(M_PI) * i / (kFFTSize - 1)));
  m_fpsTimer = QDateTime::currentMSecsSinceEpoch();

  m_idleTimer = new QTimer(this);
  m_idleTimer->setSingleShot(true);
  m_idleTimer->setInterval(kIdleMs);
  connect(m_idleTimer, &QTimer::timeout, this, &Cleave::onIdleTimeout);
}

Cleave::~Cleave() { stopCapture(); }

QVariantList Cleave::magnitudes() const {
  QVariantList out;
  out.reserve(m_bandCount);
  for (float v : m_mag)
    out.append(v);
  return out;
}

QVariantList Cleave::peaks() const {
  QVariantList out;
  out.reserve(m_bandCount);
  for (float v : m_pk)
    out.append(v);
  return out;
}

void Cleave::setBandCount(int n) {
  n = qBound(2, n, 512);
  if (n == m_bandCount)
    return;
  m_bandCount = n;
  m_mag.assign(n, 0.0f);
  m_pk.assign(n, 0.0f);
  emit bandCountChanged();
}

void Cleave::killProc() {
  if (!m_proc)
    return;
  m_proc->terminate();
  m_proc->waitForFinished(1000);
  delete m_proc;
  m_proc = nullptr;
}

bool Cleave::startCapture() {
  killProc();
  m_suspended = false;
  m_silent = false;
  m_readCount = 0;

  QStringList cmd = {QStringLiteral("parec"),
                     QStringLiteral("--raw"),
                     QStringLiteral("--format=float32le"),
                     QStringLiteral("--rate=48000"),
                     QStringLiteral("--channels=1"),
                     QStringLiteral("--latency-msec=50"),
                     QStringLiteral("--device=") + m_device};

  m_proc = new QProcess(this);
  connect(m_proc, &QProcess::readyReadStandardOutput, this,
          &Cleave::onReadyRead);
  connect(m_proc, &QProcess::errorOccurred, this, &Cleave::onProcessError);

  m_proc->start(cmd[0], cmd.mid(1));
  if (!m_proc->waitForStarted(3000)) {
    emit error("failed to start parec");
    delete m_proc;
    m_proc = nullptr;
    return false;
  }

  if (m_debugMode)
    qDebug("[Cleave] capture started: %s", qPrintable(cmd.join(' ')));

  m_active = true;
  emit activeChanged();
  return true;
}

void Cleave::stopCapture() {
  m_idleTimer->stop();
  killProc();
  m_pcmBuf.clear();
  m_pcmBuf.squeeze();
  m_active = false;
  m_suspended = false;
  m_silent = false;
  if (m_debugMode)
    qDebug("[Cleave] capture stopped");
  emit activeChanged();
}

void Cleave::resumeCapture() {
  if (m_debugMode)
    qDebug("[Cleave] resuming capture");
  startCapture();
  emit suspendedChanged();
}

void Cleave::reset() {
  m_pcmBuf.clear();
  m_pcmBuf.squeeze();
  m_mag.fill(0.0f);
  m_pk.fill(0.0f);
  m_silent = false;
  m_readCount = 0;
  emit dataChanged();
}

void Cleave::onReadyRead() {
  QByteArray raw = m_proc->readAllStandardOutput();
  ++m_readCount;
  if (m_debugMode && m_readCount % 100 == 0)
    qDebug("[Cleave] chunks read: %d", m_readCount);
  processPCM(raw);
}

void Cleave::onProcessError() {
  if (m_debugMode)
    qWarning("[Cleave] process error: %s",
             qPrintable(m_proc ? m_proc->errorString() : "unknown"));
  emit error("process error");
  stopCapture();
}

void Cleave::onIdleTimeout() {
  if (m_debugMode)
    qDebug("[Cleave] idle — suspending parec");
  killProc();
  m_pcmBuf.clear();
  m_pcmBuf.squeeze();
  std::fill(m_re, m_re + kFFTSize, 0.0f);
  std::fill(m_im, m_im + kFFTSize, 0.0f);
  m_active = false;
  m_suspended = true;
  m_silent = true;
  emit activeChanged();
  emit suspendedChanged();
  if (m_debugMode)
    qDebug("[Cleave] suspended — all buffers freed");
}

void Cleave::processPCM(const QByteArray &raw) {
  m_pcmBuf.append(raw);
  if (m_pcmBuf.size() > kBufCap)
    m_pcmBuf.remove(0, m_pcmBuf.size() - kBufCap);

  int frames = 0;
  bool anySound = false;

  while (m_pcmBuf.size() >= kFrameBytes) {
    std::memcpy(m_re, m_pcmBuf.constData(), kFrameBytes);
    m_pcmBuf.remove(0, kHopBytes);

    float rms = 0.0f;
    for (int i = 0; i < kFFTSize; ++i)
      rms += m_re[i] * m_re[i];
    rms = sqrtf(rms / kFFTSize);
    if (rms < m_silenceThreshold) {
      ++frames;
      continue;
    }
    anySound = true;
    runFFT();
    ++frames;
  }

  if (anySound) {
    m_silent = false;
    m_suspended = false;
    m_idleTimer->start();
    if (m_debugMode)
      qDebug("[Cleave] processed %d frames (sound), buf: %lld bytes", frames,
             static_cast<long long>(m_pcmBuf.size()));
    emit dataChanged();
    return;
  }

  if (m_silent)
    return;

  bool stillActive = false;
  for (int b = 0; b < m_bandCount; ++b) {
    m_mag[b] = qMax(0.0f, m_mag[b] - m_peakDecay);
    m_pk[b] = qMax(0.0f, m_pk[b] - m_peakDecay);
    if (m_mag[b] > 0.0f || m_pk[b] > 0.0f)
      stillActive = true;
  }
  emit dataChanged();
  if (!stillActive)
    m_silent = true;
}

void Cleave::runFFT() {
  for (int i = 0; i < kFFTSize; ++i) {
    m_re[i] *= m_win[i];
    m_im[i] = 0.0f;
  }
  fftInPlace(m_re, m_im, kFFTSize);

  float mag[kHalf];
  for (int i = 0; i < kHalf; ++i) {
    float re = m_re[i], im = m_im[i];
    mag[i] = sqrtf(re * re + im * im) / kHalf;
  }

  const float logMin = logf(1.0f);
  const float logMax = logf(float(kHalf - 1));
  const float step = (logMax - logMin) / m_bandCount;

  for (int b = 0; b < m_bandCount; ++b) {
    int lo = qBound(1, int(expf(logMin + b * step) + 0.5f), kHalf - 1);
    int hi = qBound(lo, int(expf(logMin + (b + 1) * step) + 0.5f), kHalf - 1);
    float sum = 0.0f;
    for (int k = lo; k <= hi; ++k)
      sum += mag[k];
    float raw = sum / (hi - lo + 1);
    float next = m_smoothing * m_mag[b] + (1.0f - m_smoothing) * raw;
    m_mag[b] = next;
    if (next >= m_pk[b])
      m_pk[b] = next;
    else
      m_pk[b] = qMax(0.0f, m_pk[b] - m_peakDecay);
  }

  ++m_frameCount;
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  if (now - m_fpsTimer >= 1000) {
    m_fps = m_frameCount;
    m_frameCount = 0;
    m_fpsTimer = now;
    if (m_debugMode)
      qDebug("[Cleave] FFT fps: %d | band[0] mag=%.4f peak=%.4f", m_fps,
             m_mag[0], m_pk[0]);
  }
}
