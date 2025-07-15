#!/bin/bash

echo "Testing SimpleOS Kernel..."
echo "Starting QEMU - you should see kernel output in the window that opens"
echo "Press Ctrl+Alt+G to grab/release mouse, Ctrl+Alt+2 for QEMU monitor, Ctrl+Alt+Q to quit"
echo ""

# Start QEMU and let user interact
qemu-system-i386 -kernel build/kernel.bin -m 128M

echo "QEMU session ended."
