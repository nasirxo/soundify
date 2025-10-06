#!/bin/bash

# Example usage script for Audio Encoder/Decoder
# This script demonstrates basic encoding and decoding operations

set -e  # Exit on error

echo "========================================"
echo "Audio Encoder/Decoder - Example Script"
echo "========================================"
echo ""

# Build the project if not already built
if [ ! -f ./audio_encoder_decoder ]; then
    echo "Building the project..."
    make clean
    make
    echo ""
fi

# Create a test file if it doesn't exist
if [ ! -f examples/test.txt ]; then
    mkdir -p examples
    echo "Creating test file..."
    cat > examples/test.txt << 'EOF'
Hello from Audio Encoder/Decoder!

This file was encoded into audio and decoded back.
Amazing, right?
EOF
fi

echo "Step 1: Encoding test.txt to audio..."
./audio_encoder_decoder encode examples/test.txt example_output.wav
echo ""

echo "Step 2: Decoding audio back to file..."
./audio_encoder_decoder decode example_output.wav ./
echo ""

echo "Step 3: Verifying the decoded file..."
if cmp -s examples/test.txt test.txt; then
    echo "✓ SUCCESS! The decoded file matches the original!"
else
    echo "✗ WARNING: The files differ. There may be some data corruption."
fi
echo ""

echo "Files created:"
echo "  - example_output.wav (encoded audio)"
echo "  - test.txt (decoded file)"
echo ""

echo "You can play example_output.wav and hear the encoded data!"
echo "Try recording it with your phone and decoding the recording."
echo ""

echo "Cleanup: rm example_output.wav test.txt"
