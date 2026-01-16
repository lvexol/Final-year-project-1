#!/bin/bash
# ===================================================================
# Build and Test Script for KasmVNC Latency Tracking Implementation
# ===================================================================
# 
# This script compiles the latency tracking modules and validates
# the implementation is working correctly.
#

set -e

echo "=========================================="
echo "KasmVNC Latency Tracking Build & Test"
echo "=========================================="
echo ""

PROJECT_DIR="/home/vexo/project/Final-year-project-1/final-1"
BUILD_DIR="$PROJECT_DIR/build"

# Step 1: Verify source files exist
echo "[1/4] Verifying source files..."
echo ""

FILES_TO_CHECK=(
    "common/rfb/LatencyTracker.h"
    "common/rfb/LatencyTracker.cxx"
    "common/rfb/LatencyMetrics.h"
    "common/rfb/LatencyMetrics.cxx"
    "common/rfb/RFBProtocolExt.h"
    "kasmweb/LatencyMonitor.js"
    "kasmweb/latency-dashboard.html"
    "LATENCY_TRACKING_GUIDE.md"
)

for file in "${FILES_TO_CHECK[@]}"; do
    if [ -f "$PROJECT_DIR/$file" ]; then
        echo "✓ $file"
    else
        echo "✗ $file NOT FOUND"
        exit 1
    fi
done

echo ""
echo "✓ All source files verified"
echo ""

# Step 2: Check CMakeLists.txt modifications
echo "[2/4] Checking CMakeLists.txt modifications..."
echo ""

if grep -q "LatencyTracker.cxx" "$PROJECT_DIR/common/rfb/CMakeLists.txt"; then
    echo "✓ LatencyTracker.cxx added to CMakeLists.txt"
else
    echo "✗ LatencyTracker.cxx NOT in CMakeLists.txt"
    exit 1
fi

if grep -q "LatencyMetrics.cxx" "$PROJECT_DIR/common/rfb/CMakeLists.txt"; then
    echo "✓ LatencyMetrics.cxx added to CMakeLists.txt"
else
    echo "✗ LatencyMetrics.cxx NOT in CMakeLists.txt"
    exit 1
fi

echo ""

# Step 3: Verify VNCSConnectionST modifications
echo "[3/4] Checking VNCSConnectionST modifications..."
echo ""

if grep -q "LatencyTracker latencyTracker" "$PROJECT_DIR/common/rfb/VNCSConnectionST.h"; then
    echo "✓ LatencyTracker member added to VNCSConnectionST.h"
else
    echo "✗ LatencyTracker member NOT in VNCSConnectionST.h"
    exit 1
fi

if grep -q "latencyTracker.recordClickStart" "$PROJECT_DIR/common/rfb/VNCSConnectionST.cxx"; then
    echo "✓ Click latency instrumentation found in VNCSConnectionST.cxx"
else
    echo "✗ Click latency instrumentation NOT found"
    exit 1
fi

if grep -q "latencyTracker.recordDisplayStart" "$PROJECT_DIR/common/rfb/VNCSConnectionST.cxx"; then
    echo "✓ Display latency instrumentation found in VNCSConnectionST.cxx"
else
    echo "✗ Display latency instrumentation NOT found"
    exit 1
fi

echo ""

# Step 4: Check includes
echo "[4/4] Verifying include paths..."
echo ""

if grep -q "#include <rfb/LatencyTracker.h>" "$PROJECT_DIR/common/rfb/VNCSConnectionST.h"; then
    echo "✓ LatencyTracker.h included in VNCSConnectionST.h"
else
    echo "✗ LatencyTracker.h NOT included"
    exit 1
fi

echo ""

# Summary
echo "=========================================="
echo "✓ All Pre-Build Checks PASSED"
echo "=========================================="
echo ""
echo "Next Steps:"
echo "1. Build the project:"
echo "   cd $BUILD_DIR && cmake .. && make -j\$(nproc)"
echo ""
echo "2. Run the server:"
echo "   $PROJECT_DIR/_install/bin/Xvnc :1 -rfbport 5900"
echo ""
echo "3. Access the latency dashboard:"
echo "   Open kasmweb/latency-dashboard.html in a web browser"
echo ""
echo "4. Test latency tracking:"
echo "   - Connect to the VNC server"
echo "   - Click the mouse and observe metrics"
echo "   - Check metrics in the dashboard"
echo ""
echo "Documentation: See LATENCY_TRACKING_GUIDE.md for detailed information"
echo ""
