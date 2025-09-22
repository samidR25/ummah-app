-- Initial database setup for Ummah App
-- This script runs when PostgreSQL container starts for the first time

-- Create extension for UUID generation
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Create enum types
CREATE TYPE user_gender AS ENUM ('male', 'female');
CREATE TYPE community_type AS ENUM ('public', 'private', 'invite_only');
CREATE TYPE message_type AS ENUM ('text', 'image', 'file', 'islamic_reminder');

-- Users table
CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    gender user_gender NOT NULL,
    location VARCHAR(255),
    islamic_verification_level INTEGER DEFAULT 0,
    prayer_reminders BOOLEAN DEFAULT true,
    gender_interaction_preference VARCHAR(50) DEFAULT 'same_gender_only',
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Communities table
CREATE TABLE IF NOT EXISTS communities (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    name VARCHAR(100) UNIQUE NOT NULL,  -- Added UNIQUE constraint
    description TEXT,
    type community_type DEFAULT 'public',
    islamic_focus VARCHAR(100),
    moderator_required BOOLEAN DEFAULT true,
    created_by UUID REFERENCES users(id),
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Community memberships
CREATE TABLE IF NOT EXISTS community_memberships (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    community_id UUID REFERENCES communities(id) ON DELETE CASCADE,
    role VARCHAR(20) DEFAULT 'member',
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(user_id, community_id)
);

-- Messages table
CREATE TABLE IF NOT EXISTS messages (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    sender_id UUID REFERENCES users(id) ON DELETE CASCADE,
    community_id UUID REFERENCES communities(id) ON DELETE CASCADE,
    receiver_id UUID REFERENCES users(id) ON DELETE CASCADE,
    content TEXT NOT NULL,
    message_type message_type DEFAULT 'text',
    is_moderated BOOLEAN DEFAULT false,
    moderation_status VARCHAR(20) DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create indexes for better performance
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_messages_sender ON messages(sender_id);
CREATE INDEX IF NOT EXISTS idx_messages_community ON messages(community_id);
CREATE INDEX IF NOT EXISTS idx_community_memberships_user ON community_memberships(user_id);

-- Insert initial development data
INSERT INTO users (username, email, password_hash, gender) VALUES 
('admin', 'admin@ummahapp.com', '$2a$10$placeholder_hash', 'male'),
('test_user', 'test@ummahapp.com', '$2a$10$placeholder_hash', 'female')
ON CONFLICT (username) DO NOTHING;

INSERT INTO communities (name, description, type, islamic_focus, created_by) VALUES 
('General Discussion', 'General Islamic discussions and community support', 'public', 'general', (SELECT id FROM users WHERE username = 'admin')),
('Quran Study', 'Quran study and discussion group', 'public', 'quran_study', (SELECT id FROM users WHERE username = 'admin')),
('Youth Group', 'Islamic youth discussions and activities', 'private', 'youth', (SELECT id FROM users WHERE username = 'admin'))
ON CONFLICT (name) DO NOTHING;

-- Success message
SELECT 'Ummah App database initialized successfully!' as status;
