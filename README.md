A Qt/QML plugin for capturing system audio.

Cleave exposes magnitudes and peaks  which are updated on every dataChanged signal. magnitudes gives you the current smoothed amplitudes/band, peaks gives you the hold value before fallout. Just listen to dataChanged, grab cleave.magnitudes, and draw whatever you want.
[example usage](https://github.com/RyuZinOh/.dotfiles/blob/ca75521c5c0dc0a498c9050372f87eed237831e3/quickshell/Modules/TopJesus/CleaveViz/CleaveViz.qml)

## here are the signals to concern when using:
<table border=1>
  <thead>
    <th><strong>Signal</strong></th>
    <th><strong>When it fires</strong></th>
  </thead>
  <tr>
    <td>dataChanged()</td>
    <td>Every time magnitudes and peaks are updated</td>
  </tr>
  <tr>
    <td>activeChanged()</td>
    <td>When capture starts or stops</td>
  </tr>
  <tr>
    <td>suspendedChanged()</td>
    <td>When idle suspension state changes</td>
  </tr>
  <tr>
    <td>bandCountChanged()</td>
    <td>When bandCount is changed</td>
  </tr>
  <tr>
    <td>peakDecayChanged()</td>
    <td>When peakDecay is changed</td>
  </tr>
  <tr>
    <td>smoothingChanged()</td>
    <td>When smoothing is changed</td>
  </tr>
  <tr>
    <td>silenceThresholdChanged()</td>
    <td>When silenceThreshold is changed</td>
  </tr>
  <tr>
    <td>debugModeChanged()</td>
    <td>When debugMode is changed</td>
  </tr>
  <tr>
    <td>deviceChanged()</td>
    <td>When device is changed</td>
  </tr>
  <tr>
    <td>error(msg)</td>
    <td>On process failure or bad configuration</td>
  </tr>
</table>

## args
<table border = 1>
  <thead>
    <tr>
      <th>Argument</th>
      <th>Type</th>
      <th>Range</th>
      <th>Description</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>bandCount</code></td>
      <td>int</td>
      <td>2 to 512</td>
      <td>Number of frequency bands</td>
    </tr>
    <tr>
      <td><code>smoothing</code></td>
      <td>float</td>
      <td>0.0 to 0.99</td>
      <td>Inter-frame averaging, higher = more fluid</td>
    </tr>
    <tr>
      <td><code>peakDecay</code></td>
      <td>float</td>
      <td>0.001 to 0.5</td>
      <td>Peak fall speed per frame, lower = slower</td>
    </tr>
    <tr>
      <td><code>silenceThreshold</code></td>
      <td>float</td>
      <td>0.0 to 1.0</td>
      <td>RMS silence cutoff, lower = more sensitive</td>
    </tr>
    <tr>
      <td><code>device</code></td>
      <td>string</td>
      <td></td>
      <td>PulseAudio source name, omit for default</td>
    </tr>
    <tr>
      <td><code>debugMode</code></td>
      <td>bool</td>
      <td></td>
      <td>Enables qDebug logging, false in production</td>
    </tr>
  </tbody>
</table>

# how to install?
`` paru -S cleave``
 begin the usage!!

