# KasmVNC Latency Tracking Implementation Guide

## Overview

This document describes the latency tracking feature added to KasmVNC, which measures:
- **Click Latency**: Time from mouse click input to effect on server
- **Display Latency**: Time from server framebuffer change to update being sent to client

## Architecture

### Backend Components

#### 1. **LatencyTracker.h / LatencyTracker.cxx**
Core latency measurement module that:
- Tracks click events from start (client input) to completion (server effect)
- Tracks display events from start (framebuffer change) to completion (update sent)
- Maintains statistics (min, max, average)
- Provides thread-safe operations with mutex locking
- Prunes old records to avoid memory bloat

**Key Methods:**
- `recordClickStart(clickId, x, y)` - Mark when click starts
- `recordClickComplete(clickId)` - Mark when click effect appears on server
- `recordDisplayStart(frameId)` - Mark when display update begins
- `recordDisplayComplete(frameId)` - Mark when update is sent to client
- `getStatistics()` - Get aggregated metrics

#### 2. **LatencyMetrics.h / LatencyMetrics.cxx**
Export module for metrics that supports:
- **JSON Export**: Full metrics and detailed records
- **CSV Export**: Historical data in tabular format
- **Statistics Export**: Human-readable statistics report

**Usage:**
```cpp
LatencyMetrics::saveToFile(tracker, "metrics.json", "json");
LatencyMetrics::saveToFile(tracker, "metrics.csv", "csv");
std::string json = LatencyMetrics::exportToJSON(tracker);
```

#### 3. **RFBProtocolExt.h**
RFB Protocol extensions defining:
- `LatencyStatsMessage` (type 253): Server→Client message with latency statistics
- `RequestLatencyStatsMessage` (type 254): Optional client→server request for stats

Message structure includes:
- Last click/display latencies
- Average latencies
- Min/max values
- Sample counts
- Unix timestamp

### Instrumentation Points

#### In VNCSConnectionST.cxx

**pointerEvent() method:**
```cpp
// At start: Record click start
latencyTracker.recordClickStart(clickId++, pos.x, pos.y);

// After desktop->pointerEvent(): Record click completion
latencyTracker.recordClickComplete(clickId - 1);
```

**writeFramebufferUpdate() method:**
```cpp
// At start: Record display update start
latencyTracker.recordDisplayStart(frameId++);

// After sock->cork(false): Record display completion
latencyTracker.recordDisplayComplete(frameId);
```

### Frontend Components

#### 1. **LatencyMonitor.js**
Client-side monitoring class providing:
- Real-time latency metric collection
- Statistical analysis (min, max, avg)
- Automatic UI updates
- Export capabilities (JSON, CSV)
- Configurable update intervals

**Usage:**
```javascript
const monitor = new LatencyMonitor({
  updateInterval: 1000,
  maxHistorySize: 100,
  enabled: true
});
monitor.init();

// Record measurements
monitor.recordClickLatency(25.5);
monitor.recordDisplayLatency(45.2);

// Get statistics
const stats = monitor.getStatistics();

// Export data
const json = monitor.exportJSON();
const csv = monitor.exportCSV();
```

#### 2. **latency-dashboard.html**
Web dashboard displaying:
- Real-time click and display latency metrics
- Color-coded status indicators:
  - Green (< 50ms): Excellent
  - Yellow (50-100ms): Acceptable
  - Red (> 100ms): Poor
- Controls for start/stop monitoring
- Export and clear functions
- Responsive design with system monitor aesthetic

## Integration Steps

### 1. Build Backend Components

```bash
cd /home/vexo/project/Final-year-project-1/final-1
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

The CMakeLists.txt has been updated to include:
- LatencyTracker.cxx
- LatencyMetrics.cxx

### 2. Verify Compilation

After building, verify no errors in:
- LatencyTracker.cxx compilation
- LatencyMetrics.cxx compilation
- VNCSConnectionST linking with LatencyTracker

### 3. Enable Frontend Monitoring

To use the latency dashboard:

1. Copy `LatencyMonitor.js` to your web server
2. Access `latency-dashboard.html` via web browser
3. Click "Start Monitoring" to begin collecting metrics
4. View real-time statistics in the metrics cards

### 4. Access Latency Data

#### Via C++ API
```cpp
// In VNCSConnectionST or other server components
LatencyTracker::Statistics stats = latencyTracker.getStatistics();
double clickLatency = stats.avgClickLatency;
double displayLatency = stats.avgDisplayLatency;

// Export metrics
LatencyMetrics::saveToFile(latencyTracker, "/var/log/kasmvnc/latency.json", "json");
```

#### Via JavaScript API
```javascript
const monitor = new LatencyMonitor();
monitor.init();

// Periodically fetch and display metrics
setInterval(() => {
  const stats = monitor.getStatistics();
  console.log('Click Latency:', stats.avgClickLatency, 'ms');
  console.log('Display Latency:', stats.avgDisplayLatency, 'ms');
}, 1000);
```

## Data Flow

### Click Latency Flow
```
Client sends mouse event
        ↓
Network transmission to server
        ↓
pointerEvent() receives click
    ↓
recordClickStart(clickId)    [START TIME]
    ↓
Process mouse input
    ↓
desktop->pointerEvent() executes
    ↓
recordClickComplete(clickId) [END TIME]
    ↓
Latency = END - START
```

### Display Latency Flow
```
Framebuffer change detected
    ↓
writeFramebufferUpdate() called
    ↓
recordDisplayStart(frameId)  [START TIME]
    ↓
Encode framebuffer regions
    ↓
Create update messages
    ↓
sock->cork(false) sends data  [END TIME]
    ↓
recordDisplayComplete(frameId)
    ↓
Latency = END - START
```

## File Locations

| File | Path | Purpose |
|------|------|---------|
| LatencyTracker.h | common/rfb/ | Core tracker interface |
| LatencyTracker.cxx | common/rfb/ | Tracker implementation |
| LatencyMetrics.h | common/rfb/ | Export interface |
| LatencyMetrics.cxx | common/rfb/ | Export implementation |
| RFBProtocolExt.h | common/rfb/ | Protocol message definitions |
| VNCSConnectionST.h | common/rfb/ | Added latencyTracker member |
| VNCSConnectionST.cxx | common/rfb/ | Instrumentation points |
| CMakeLists.txt | common/rfb/ | Build configuration |
| LatencyMonitor.js | kasmweb/ | Frontend monitoring class |
| latency-dashboard.html | kasmweb/ | Web dashboard UI |

## Configuration

### Update Intervals

Modify in LatencyMonitor.js or at runtime:
```javascript
const monitor = new LatencyMonitor({
  updateInterval: 1000,        // Update UI every 1 second
  maxHistorySize: 100,         // Keep 100 recent measurements
  enabled: true
});
```

### Log Output

C++ backend logs latency events:
```
Click X latency: 23.45 ms at (512, 300)
Display Y latency: 15.67 ms
```

Enable debug logging:
```cpp
// In code: vlog.debug() statements will output when logger level is set
// Via environment: VNC_LOG_LEVEL=DEBUG
```

## Performance Considerations

1. **Memory Usage**: Limited by `maxHistorySize` (default 100 records)
2. **CPU Overhead**: Minimal - only timestamp collection and arithmetic
3. **Network**: No additional network traffic (local measurement only)
4. **Mutex Contention**: Brief locks during record and stat updates

## Troubleshooting

### No Latency Data Showing

1. Verify LatencyTracker module compiled successfully
2. Check that pointerEvent() and writeFramebufferUpdate() are being called
3. Enable verbose logging to confirm measurement points

### High Latency Values

- May indicate network congestion
- Could be due to server load
- Check framebuffer encoding performance
- Verify client/server network connectivity

### Frontend Not Displaying

1. Ensure LatencyMonitor.js is loaded before latency-dashboard.html
2. Check browser console for JavaScript errors
3. Verify CORS settings if loading from different domain

## Future Enhancements

Possible extensions:
1. **Real-time graphing** with Chart.js integration
2. **Network simulation** to test under adverse conditions
3. **Performance tuning** recommendations based on metrics
4. **Persistent storage** of historical latency trends
5. **Alerting** when latency exceeds thresholds
6. **Per-client tracking** for multi-user scenarios
7. **Integration with monitoring systems** (Prometheus, Grafana)

## Testing

### Manual Testing

```bash
# Build with latency tracking
make -j$(nproc)

# Run KasmVNC server
./bin/Xvnc :1 -rfbport 5900

# Connect client and monitor
# Open latency-dashboard.html in browser
# Perform mouse clicks and observe metrics
```

### Automated Testing

Create test scenario:
```cpp
// Simulate click events
for (int i = 0; i < 100; i++) {
  tracker.recordClickStart(i, 100, 100);
  // Simulate 20ms delay
  usleep(20000);
  tracker.recordClickComplete(i);
}
```

## Documentation

- See [LatencyTracker.h](common/rfb/LatencyTracker.h) for API documentation
- See [LatencyMonitor.js](kasmweb/LatencyMonitor.js) for frontend API
- See [RFBProtocolExt.h](common/rfb/RFBProtocolExt.h) for protocol details

## License

All latency tracking components are licensed under GNU General Public License v2.0 or later, consistent with KasmVNC licensing.

---

**Implementation Date**: January 16, 2026  
**Version**: 1.0  
**Status**: Complete and Ready for Integration
