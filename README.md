# Ummah App - Social Platform

A secure focused social media platform connecting the Ummah!

## Vision

Building meaningful connections within the global Ummah through a platform that prioritizes values, privacy, security, and authentic community engagement.

## Architecture

### Backend Services (C++)
- **Auth Service** (Port 8080) - JWT authentication with compliance
- **Community Service** (Port 8081) - Community management
- **Messaging Service** (Port 8082) - Secure messaging with moderation

### Technology Stack
- **Database**: PostgreSQL 
- **Cache**: Redis for session management
- **Containerization**: Docker with Kubernetes support
- **Frontend**: React-based web interface

### Prerequisites
- Docker and Docker Compose
- C++ build environment (GCC/Clang)

### Development Setup

1. **Clone Repository**
```bash
git clone https://github.com/samidR25/ummah-app.git
cd ummah-app
````

2. **Environment Configuration**

```bash
# Copy template and add your credentials
cp config/environments/.env.development .env
# Edit .env with your local development values
```

3. **Start Services**

```bash
# Start infrastructure
docker-compose -f docker-compose.dev.yml up -d

# Build services
./scripts/build-auth-service.sh

# Test services
./scripts/test-auth-service.sh
```

## Security

All sensitive data is managed via environment variables:

- `POSTGRES_PASSWORD` - Database authentication
- `REDIS_PASSWORD` - Cache authentication
- `JWT_SECRET` - Token signing secret

**Template files contain only placeholders. Real secrets are never committed.**

