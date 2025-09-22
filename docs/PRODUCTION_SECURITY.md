# Production Security Checklist

## Pre-Deployment Security
- [ ] All passwords are strong (min 32 characters, mixed case, numbers, symbols)
- [ ] .env.production is NOT in git (check .gitignore)
- [ ] Database passwords are unique per environment
- [ ] JWT secrets are cryptographically secure (64+ characters)
- [ ] Encryption keys are properly generated (32 bytes for AES-256)

## Database Security
- [ ] PostgreSQL user has minimal required permissions
- [ ] Database is not exposed to public internet
- [ ] SSL/TLS enabled for database connections
- [ ] Regular automated backups configured
- [ ] Database logs are monitored

## Application Security
- [ ] All HTTP traffic redirected to HTTPS
- [ ] Strong SSL/TLS certificates installed
- [ ] CORS properly configured
- [ ] Rate limiting implemented
- [ ] Input validation on all endpoints
- [ ] SQL injection prevention (parameterized queries)

## Infrastructure Security
- [ ] Firewall rules configured (only necessary ports open)
- [ ] SSH key-based authentication (no password login)
- [ ] Regular security updates applied
- [ ] Monitoring and alerting configured
- [ ] Log retention policy implemented

## Islamic Compliance Security
- [ ] Content moderation systems active
- [ ] Scholar review board access secured
- [ ] User privacy controls implemented
- [ ] Data retention follows Islamic principles
- [ ] Audit trail for content decisions
