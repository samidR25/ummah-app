#!/bin/bash

ENVIRONMENT=${1:-development}

echo "Setting up $ENVIRONMENT environment..."

if [ "$ENVIRONMENT" = "production" ]; then
    if [ ! -f "config/environments/.env.production" ]; then
        echo "Creating production environment from template..."
        cp config/environments/.env.production.template config/environments/.env.production
        echo "⚠️  IMPORTANT: Edit .env.production with real values!"
        echo "⚠️  NEVER commit .env.production to git!"
        exit 1
    fi
fi

# Start appropriate docker-compose
if [ "$ENVIRONMENT" = "production" ]; then
    docker-compose -f docker-compose.prod.yml up -d
elif [ "$ENVIRONMENT" = "staging" ]; then
    docker-compose -f docker-compose.staging.yml up -d
else
    docker-compose -f docker-compose.dev.yml up -d
fi

echo "✅ $ENVIRONMENT environment started!"
