#!/bin/bash

echo "📦 Installing dependencies for Ummah App Auth Service..."

# Update system packages
sudo apt update

# Install build essentials
echo "🔧 Installing build tools..."
sudo apt install -y build-essential cmake git pkg-config curl

# Install C++ libraries
echo "📚 Installing C++ libraries..."
sudo apt install -y libssl-dev libpq-dev nlohmann-json3-dev

# Check if Crow framework is installed
echo "🐦 Checking Crow framework..."
if ! ldconfig -p | grep -q libCrow; then
    echo "Installing Crow framework..."
    
    # Remove any previous installation attempts
    sudo rm -rf /tmp/crow
    
    cd /tmp
    git clone --depth 1 https://github.com/CrowCpp/Crow.git crow
    cd crow
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with minimal features for faster build
    cmake .. \
        -DCROW_BUILD_EXAMPLES=OFF \
        -DCROW_BUILD_TESTS=OFF \
        -DCROW_BUILD_DOCS=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local
    
    # Build with all available cores
    make -j$(nproc)
    
    # Install
    sudo make install
    sudo ldconfig
    
    echo "✅ Crow framework installed"
else
    echo "ℹ️  Crow framework already installed"
fi

# Verify installations
echo "🔍 Verifying installations..."

# Check pkg-config files
if pkg-config --exists libpq; then
    echo "✅ PostgreSQL development libraries: $(pkg-config --modversion libpq)"
else
    echo "❌ PostgreSQL development libraries not found"
fi

if [ -f "/usr/include/nlohmann/json.hpp" ]; then
    echo "✅ nlohmann/json library found"
else
    echo "❌ nlohmann/json library not found"
fi

if ldconfig -p | grep -q libssl; then
    echo "✅ OpenSSL libraries found"
else
    echo "❌ OpenSSL libraries not found"
fi

if ldconfig -p | grep -q libCrow; then
    echo "✅ Crow framework found"
else
    echo "❌ Crow framework not found"
fi

echo ""
echo "✅ Dependencies installation completed!"
echo "🔨 You can now run: ./scripts/build-auth-service.sh"