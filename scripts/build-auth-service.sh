#!/bin/bash

echo "🔨 Building Ummah Auth Service..."

# Get project root directory
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
echo "📁 Project root: $PROJECT_ROOT"

cd "$PROJECT_ROOT"

# Check if we're in the right directory
if [ ! -f "backend/auth-service/CMakeLists.txt" ]; then
    echo "❌ Error: CMakeLists.txt not found in backend/auth-service/"
    echo "Please run this script from the project root directory"
    exit 1
fi

# Check for required files
echo "🔍 Checking required files..."
REQUIRED_FILES=(
    "backend/common/include/config.h"
    "backend/auth-service/include/database_manager.h"
    "backend/auth-service/include/password_manager.h"
    "backend/auth-service/include/jwt_manager.h"
    "backend/auth-service/include/auth_controller.h"
    "backend/auth-service/src/main.cpp"
    "backend/auth-service/src/database_manager.cpp"
    "backend/auth-service/src/password_manager.cpp"
    "backend/auth-service/src/jwt_manager.cpp"
    "backend/auth-service/src/auth_controller.cpp"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Required file missing: $file"
        exit 1
    fi
done
echo "✅ All required files found"

# Navigate to auth service directory - THIS WAS THE BUG!
cd backend/auth-service

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf build
mkdir -p build
cd build

# FIXED: Run cmake from the auth-service/build directory, pointing to parent directory
echo "⚙️  Configuring CMake..."
echo "📁 Current directory: $(pwd)"
echo "📁 CMakeLists.txt location: $(pwd)/../CMakeLists.txt"

cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    echo ""
    echo "🔧 Common solutions:"
    echo "   1. Install dependencies: sudo apt install nlohmann-json3-dev"
    echo "   2. Install Crow framework: run scripts/install-dependencies.sh"
    echo "   3. Check if PostgreSQL dev libraries installed: sudo apt install libpq-dev"
    exit 1
fi

# Build
echo "🏗️  Building..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📁 Binary location: $(pwd)/ummah_auth_service"
    echo ""
    echo "🚀 Next steps:"
    echo "   1. Start database: docker-compose -f ../../../docker-compose.dev.yml up -d"
    echo "   2. Run service: ./ummah_auth_service"
    echo "   3. Test health: curl http://localhost:8080/health"
    echo "   4. Run full tests: ../../../scripts/test-auth-service.sh"
else
    echo "❌ Build failed!"
    echo "Check the error messages above for details."
    exit 1
fi