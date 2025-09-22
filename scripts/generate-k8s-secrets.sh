#!/bin/bash
# Helper script to generate Kubernetes secrets

echo "Generating Kubernetes secrets..."

# Read from production environment file
if [ ! -f "config/environments/.env.production" ]; then
    echo "Error: .env.production not found!"
    echo "Copy .env.production.template and fill with real values"
    exit 1
fi

source config/environments/.env.production

# Generate base64 encoded secrets
POSTGRES_PASSWORD_B64=$(echo -n "$POSTGRES_PASSWORD" | base64)
POSTGRES_USER_B64=$(echo -n "$POSTGRES_USER" | base64)
REDIS_PASSWORD_B64=$(echo -n "$REDIS_PASSWORD" | base64)
JWT_SECRET_B64=$(echo -n "$JWT_SECRET" | base64)
ENCRYPTION_KEY_B64=$(echo -n "$ENCRYPTION_KEY" | base64)

# Create the actual secrets file
cat > infrastructure/kubernetes/secrets/database-secrets.yml << EOF_INNER
apiVersion: v1
kind: Secret
metadata:
  name: ummah-app-secrets
  namespace: ummah-app
type: Opaque
data:
  postgres-password: $POSTGRES_PASSWORD_B64
  postgres-user: $POSTGRES_USER_B64
  redis-password: $REDIS_PASSWORD_B64
  jwt-secret: $JWT_SECRET_B64
  encryption-key: $ENCRYPTION_KEY_B64
EOF_INNER

echo "✅ Kubernetes secrets generated in infrastructure/kubernetes/secrets/database-secrets.yml"
echo "⚠️  Remember: NEVER commit this file to git!"
