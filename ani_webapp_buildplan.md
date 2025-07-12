# Ani Web Application Build Plan

## Executive Summary

This document outlines a comprehensive plan to build a production-ready web application called "OtakuTrack". The
web app will have modern web features including user authentication, blog functionality, and a sleek React-based
interface.

### App Overview

- **Core Features**: Today's anime schedule, weekday filtering, anime search
- **Data Models**: Anime, Episode, SeasonSchedule structures
- **API Integration**: Jikan REST API v4 for MyAnimeList data
- **Architecture**: Modular C codebase with network, parsing, filtering, and display layers
- **Key Endpoints**:
  - `/seasons/now` - Current season anime
  - `/schedules?filter={weekday}` - Weekday schedules
  - `/anime?q={title}` - Anime search

## Project Scope & Objectives

### Primary Goals

1. Implement the relavant functionality for a modern web application
2. Implement user authentication and blog features
3. Create responsive, mobile-friendly interface
4. Ensure production-grade security and performance
5. Deploy to Vercel with custom domain (www.otakutrack.com)

### Key Features

- **Authentication**: Login/logout with 7-day cookie persistence
- **Home Page**: Dynamic anime schedule search and display
- **Blog System**: User-generated anime blog posts with rich text editing
- **Responsive Design**: Mobile-first, desktop-optimized interface
- **Security**: Protection against common web vulnerabilities

---

## Phase 1: Foundation & Backend Setup

### 1.1 Project Initialization

- **Create new project structure**

  ```
  otaku-track/
  ├── backend/
  │   ├── src/
  │   │   ├── routes/
  │   │   ├── middleware/
  │   │   ├── models/
  │   │   ├── controllers/
  │   │   ├── services/
  │   │   └── utils/
  │   ├── database/
  │   │   ├── migrations/
  │   │   └── seeds/
  │   └── tests/
  ├── frontend/
  │   ├── public/
  │   ├── src/
  │   │   ├── components/
  │   │   ├── pages/
  │   │   ├── hooks/
  │   │   ├── services/
  │   │   ├── utils/
  │   │   └── styles/
  │   └── tests/
  └── docs/
  ```

- **Initialize git repository**
- **Set up package.json files for both frontend and backend**
- **Configure TypeScript for type safety**

### 1.2 Backend Technology Stack

- **Runtime**: Node.js with Express.js
- **Language**: TypeScript for type safety
- **Database**: SQLite with better-sqlite3 driver
- **Authentication**: JWT tokens with secure HTTP-only cookies
- **Security**: Helmet, CORS, rate limiting, input validation
- **API Documentation**: OpenAPI/Swagger
- **Testing**: Jest + Supertest

### 1.3 Database Schema Design

```sql
-- Users table
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT 1
);

-- Blog posts table
CREATE TABLE blog_posts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    title VARCHAR(255) NOT NULL,
    content TEXT NOT NULL,
    anime_title VARCHAR(255),
    anime_mal_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    is_published BOOLEAN DEFAULT 1,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Anime cache table (for performance)
CREATE TABLE anime_cache (
    mal_id INTEGER PRIMARY KEY,
    title VARCHAR(512) NOT NULL,
    studio VARCHAR(64),
    weekday VARCHAR(10),
    type VARCHAR(10),
    episodes INTEGER,
    status VARCHAR(20),
    cached_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    expires_at DATETIME NOT NULL
);

-- Daily schedule cache
CREATE TABLE schedule_cache (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    weekday VARCHAR(10) NOT NULL,
    season VARCHAR(8) NOT NULL,
    year INTEGER NOT NULL,
    data TEXT NOT NULL, -- JSON string
    cached_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    expires_at DATETIME NOT NULL
);

-- User sessions (for session management)
CREATE TABLE user_sessions (
    id VARCHAR(128) PRIMARY KEY,
    user_id INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    expires_at DATETIME NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);
```

### 1.4 Core Backend Services

- **AnimeService**: Needed web services
  - `getCurrentSeasonSchedule()`
  - `getWeekdaySchedule(weekday: string)`
  - `searchAnime(query: string)`
  - Implement caching layer for performance

- **AuthService**: User authentication and session management
  - `register(username, email, password)`
  - `login(email, password)`
  - `logout(sessionId)`
  - `validateSession(sessionId)`

- **BlogService**: Blog post management
  - `createPost(userId, title, content, animeInfo)`
  - `updatePost(postId, updates)`
  - `deletePost(postId)`
  - `getPost(postId)`
  - `getUserPosts(userId)`
  - `getAllPosts(pagination)`

### 1.5 API Route Structure

```
/api/v1/
├── auth/
│   ├── POST /register
│   ├── POST /login
│   ├── POST /logout
│   └── GET /verify
├── anime/
│   ├── GET /schedule/today
│   ├── GET /schedule/:weekday
│   ├── GET /search?q={query}
│   └── GET /season/current
├── blog/
│   ├── GET /posts
│   ├── GET /posts/:id
│   ├── POST /posts
│   ├── PUT /posts/:id
│   ├── DELETE /posts/:id
│   └── GET /users/:userId/posts
└── user/
    ├── GET /profile
    └── PUT /profile
```

### 1.6 Security Implementation

- **Input Validation**: Joi or Zod schema validation
- **Rate Limiting**: Express-rate-limit with Redis if needed
- **CORS Configuration**: Restrict to frontend domain
- **Security Headers**: Helmet.js for security headers
- **SQL Injection Prevention**: Parameterized queries
- **XSS Prevention**: Content sanitization
- **CSRF Protection**: SameSite cookies + CSRF tokens
- **Password Security**: bcrypt with salt rounds
- **Session Security**: Secure, HTTP-only cookies with proper expiration

---

## Phase 2: Frontend Foundation

### 2.1 Frontend Technology Stack

- **Framework**: React 18 with TypeScript
- **Build Tool**: Vite for fast development and building
- **Styling**: Tailwind CSS for utility-first styling
- **State Management**: React Query for server state + Zustand for client state
- **Routing**: React Router v6
- **Forms**: React Hook Form with Zod validation
- **Rich Text Editor**: TipTap or React Quill for blog posts
- **HTTP Client**: Axios with interceptors
- **Testing**: Vitest + React Testing Library
- **UI Components**: Headless UI or Radix UI for accessibility

### 2.2 Component Architecture

```
src/
├── components/
│   ├── common/
│   │   ├── Header/
│   │   ├── Footer/
│   │   ├── Navigation/
│   │   ├── SearchBar/
│   │   ├── Table/
│   │   └── Modal/
│   ├── auth/
│   │   ├── LoginForm/
│   │   ├── RegisterForm/
│   │   └── ProtectedRoute/
│   ├── anime/
│   │   ├── AnimeTable/
│   │   ├── AnimeCard/
│   │   └── ScheduleView/
│   └── blog/
│       ├── BlogPost/
│       ├── BlogEditor/
│       ├── BlogList/
│       └── BlogCard/
├── pages/
│   ├── Home/
│   ├── Login/
│   ├── Register/
│   ├── BlogSpot/
│   ├── BlogPost/
│   ├── CreatePost/
│   └── About/
├── hooks/
│   ├── useAuth.ts
│   ├── useAnime.ts
│   ├── useBlog.ts
│   └── useLocalStorage.ts
├── services/
│   ├── api.ts
│   ├── auth.ts
│   ├── anime.ts
│   └── blog.ts
└── utils/
    ├── constants.ts
    ├── types.ts
    └── helpers.ts
```

### 2.3 Design System & Styling

- **Color Palette**: Modern anime-inspired colors
  - Primary: Deep purple/blue (#6366f1)
  - Secondary: Accent pink/red (#ec4899)
  - Neutral: Gray scale for backgrounds and text
  - Success/Warning/Error states

- **Typography**: Clean, readable font stack
  - Headings: Inter or Poppins
  - Body: System fonts for performance
  - Code: Fira Code for any technical content

- **Layout**: Mobile-first responsive design
  - Breakpoints: 640px, 768px, 1024px, 1280px
  - Grid system for consistent spacing
  - Flexbox for component layouts

### 2.4 State Management Strategy

- **Server State**: React Query for caching API responses
- **Client State**: Zustand for user preferences, UI state
- **Form State**: React Hook Form for form management
- **Authentication State**: Combination of React Query and local storage

---

## Phase 3: Authentication System

### 3.1 Frontend Authentication Components

- **Login Page**: Clean, minimalist design with email/password
- **Registration Page**: Username, email, password with validation
- **Protected Routes**: HOC for route protection
- **Authentication Context**: Global auth state management

### 3.2 Authentication Flow

1. **Registration**:
   - Form validation (email format, password strength)
   - Duplicate email/username check
   - Password hashing on backend
   - Automatic login after registration

2. **Login**:
   - Credential validation
   - JWT token generation
   - Secure cookie setting (7-day expiration)
   - Redirect to intended page

3. **Session Management**:
   - HTTP-only cookies for security
   - Automatic token refresh
   - Logout functionality
   - Session expiration handling

### 3.3 Security Features

- **Password Requirements**: 8+ characters, mixed case, numbers
- **Account Lockout**: Temporary lockout after failed attempts
- **Remember Me**: Extended session (7 days)
- **Session Invalidation**: Logout clears all sessions
- **HTTPS Only**: Force secure connections in production

---

## Phase 4: Core Anime Functionality

### 4.1 Home Page Implementation

- **Google-Style Search Bar**:
  - Auto-complete suggestions
  - Real-time search as user types
  - Search history (local storage)
  - Clear search functionality

- **Dynamic Anime Table**:
  - Default: Today's schedule on page load
  - Columns: Title, Studio, Type, Broadcast, Episodes
  - Sortable columns
  - Responsive design (stacked on mobile)
  - Loading states and error handling

### 4.2 Search Functionality

- **Search Types**:
  - Anime title search
  - Weekday filtering
  - Studio search
  - Type filtering (TV, Movie, OVA)

- **Performance Optimizations**:
  - Debounced search (300ms delay)
  - Result caching
  - Infinite scroll or pagination
  - Skeleton loading states

### 4.3 Data Integration

- **API**:REST endpoints
- **Caching Strategy**:
  - Daily schedule cached for 6 hours
  - Search results cached for 30 minutes
  - Browser caching with proper headers
- **Error Handling**: Graceful fallbacks and user feedback

---

## Phase 5: Blog System

### 5.1 Blog Architecture

- **Rich Text Editor**:
  - WYSIWYG editor (TipTap/Quill)
  - Markdown support
  - Image upload capability
  - Auto-save drafts

- **Blog Post Structure**:
  - Title with character limits
  - Rich content body
  - Anime tagging (link to MAL data)
  - Publication status (draft/published)
  - Created/updated timestamps

### 5.2 Blog Features

- **Post Management**:
  - Create/edit/delete posts
  - Draft saving
  - Preview mode
  - Publication scheduling

- **Content Organization**:
  - User post filtering
  - Anime-based categorization
  - Search within blog posts
  - Pagination or infinite scroll

### 5.3 Blog UI/UX

- **Blog List Page**: Card-based layout with excerpts
- **Individual Post View**: Clean reading experience
- **Editor Interface**: Distraction-free writing environment
- **Responsive Design**: Optimized for mobile reading

---

## Phase 6: Advanced Features & Polish

### 6.1 Performance Optimizations

- **Code Splitting**: Route-based and component-based splitting
- **Image Optimization**: WebP format, lazy loading
- **Bundle Analysis**: Webpack bundle analyzer
- **Service Worker**: Offline capability for schedules
- **CDN Integration**: Static asset delivery

### 6.2 User Experience Enhancements

- **Dark/Light Mode**: Theme switching with persistence
- **Keyboard Shortcuts**: Power user features
- **Accessibility**: WCAG 2.1 AA compliance

### 6.3 Advanced Search Features

- **Filters Panel**: Advanced filtering options
- **Search History**: Recent searches and saved searches
- **Recommendations**: Based on user activity
- **Trending**: Popular anime tracking

---

## Phase 7: Testing & Quality Assurance

### 7.1 Testing Strategy

- **Unit Tests**: 90%+ coverage for utilities and services
- **Integration Tests**: API endpoint testing
- **Component Tests**: React component behavior
- **E2E Tests**: Critical user flows (auth, search, blog)
- **Performance Tests**: Load testing for API endpoints
- **Security Tests**: Vulnerability scanning

### 7.2 Code Quality

- **ESLint/Prettier**: Code formatting and linting
- **TypeScript**: Strict type checking
- **Husky**: Pre-commit hooks for quality gates
- **SonarQube**: Code quality analysis
- **Documentation**: Comprehensive API and component docs

### 7.3 Browser Testing

- **Cross-Browser**: Chrome, Firefox, Safari, Edge
- **Mobile Testing**: iOS Safari, Chrome Mobile
- **Responsive Testing**: Various screen sizes
- **Performance Testing**: Lighthouse audits

---

## Phase 8: Deployment & Production Setup

### 8.1 Production Environment

- **Vercel Deployment**:
  - Frontend: Static site deployment
  - API Routes: Serverless functions
  - Database: Vercel Postgres or PlanetScale
  - Environment variables management

### 8.2 Domain & SSL

- **Domain Setup**: Configure www.otakutrack.com
- **SSL Certificate**: Automatic HTTPS with Vercel
- **CDN Configuration**: Global content delivery
- **DNS Management**: Proper DNS configuration

### 8.3 Monitoring & Analytics

- **Error Tracking**: Sentry for error monitoring
- **Analytics**: Privacy-focused analytics (Plausible)
- **Performance Monitoring**: Web Vitals tracking
- **Uptime Monitoring**: Status page for service health

### 8.4 Security Hardening

- **Security Headers**: HSTS, CSP, etc.
- **Rate Limiting**: API protection
- **DDoS Protection**: Cloudflare integration
- **Backup Strategy**: Database backups
- **Security Scanning**: Regular vulnerability scans

---

## Technical Architecture Details

### Frontend Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   React App     │    │   API Gateway   │    │   Database      │
│                 │    │                 │    │                 │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ Components  │ │◄──►│ │ Auth Routes │ │◄──►│ │ Users       │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ State Mgmt  │ │    │ │ Anime API   │ │◄──►│ │ Cache       │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ Routing     │ │    │ │ Blog API    │ │◄──►│ │ Blog Posts  │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Backend API Design

```typescript
// Core interfaces
interface Anime {
  mal_id: number;
  title: string;
  studio: string;
  weekday: string;
  current_episode: Episode;
  type: "TV" | "Movie" | "OVA" | "Special";
  episodes: number;
  status: string;
}

interface Episode {
  number: number;
  airdate: string;
  airtime: string;
}

interface SeasonSchedule {
  season: string;
  year: number;
  anime_list: Anime[];
  count: number;
}

interface BlogPost {
  id: number;
  user_id: number;
  title: string;
  content: string;
  anime_title?: string;
  anime_mal_id?: number;
  created_at: string;
  updated_at: string;
  is_published: boolean;
}
```

### Security Implementation

```typescript
// JWT Token Structure
interface JWTPayload {
  user_id: number;
  username: string;
  email: string;
  iat: number;
  exp: number;
}

// Session Management
interface UserSession {
  id: string;
  user_id: number;
  created_at: Date;
  expires_at: Date;
}

// Input Validation Schema
const blogPostSchema = z.object({
  title: z.string().min(1).max(255),
  content: z.string().min(1),
  anime_title: z.string().optional(),
  anime_mal_id: z.number().optional(),
  is_published: z.boolean().default(true),
});
```

---

## Conclusion

This comprehensive build plan for a modern, secure, and scalable web application. The phased
approach ensures steady progress while maintaining quality and security standards. The resulting
OtakuTrack web application will provide anime fans with a powerful tool for tracking schedules
and sharing their passion through blog posts.

The plan prioritizes:

1. **Security**: Production-grade security measures
2. **Performance**: Fast, responsive user experience
3. **Scalability**: Architecture that can grow with user base
4. **Maintainability**: Clean, well-documented codebase
5. **User Experience**: Intuitive, mobile-friendly interface

Upon completion of this plan, the web application should be ready for production deployment on
www.otakutrack.com via Vercel with all requested features implemented and thoroughly tested.

