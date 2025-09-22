#!/bin/bash

echo "📁 Creating auth service directory structure..."

cd "$(dirname "$0")/.."

# Create all required directories
mkdir -p backend/common/include
mkdir -p backend/auth-service/{src,include,tests,config,build}
mkdir -p scripts

echo "✅ Directory structure created!"
echo "📂 Next steps:"
echo "   1. Run: scripts/install-dependencies.sh"
echo "   2. Create all the source files from the artifact"
echo "   3. Run: scripts/build-auth-service.sh"