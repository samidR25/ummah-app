#!/bin/bash

echo "🔨 Building all Ummah App services..."

cd "$(dirname "$0")/.."

# Build auth service
echo "📡 Building Auth Service..."
cd backend/auth-service
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "❌ Auth service build failed!"
    exit 1
fi
cd ../../..

# Build community service
echo "🏘️ Building Community Service..."
cd backend/community-service
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "❌ Community service build failed!"
    exit 1
fi
cd ../../..

# Build messaging service
echo "💬 Building Messaging Service..."
cd backend/messaging-service
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "❌ Messaging service build failed!"
    exit 1
fi
cd ../../..

# Install frontend dependencies
echo "🌐 Setting up Frontend..."
cd frontend
npm install
cd ..

echo "✅ All services built successfully!"
echo ""
echo "🚀 To start the complete application:"
echo "   docker-compose up -d"
echo ""
echo "📊 To access the application:"
echo "   Frontend: http://localhost:3000"
echo "   Auth API: http://localhost:8080"
echo "   Community API: http://localhost:8081"
echo "   Messages API: http://localhost:8082"