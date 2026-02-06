# CLAUDE.md - AI Assistant Guide for Restaurant-Management

## Project Overview

Restaurant-Management is a greenfield project for building a restaurant management system. The repository is in its initial setup phase — no technology stack, framework, or architecture has been chosen yet.

**Repository**: thomasgpeters/Restaurant-Management
**Status**: Initial setup — no source code, dependencies, or configuration exist yet.

## Current State

The repository currently contains only:
- `README.md` — minimal project title
- `CLAUDE.md` — this file

There is no source code, build system, test framework, database schema, or CI/CD pipeline configured.

## Development Guidelines

### When Starting Development

Since this is a new project, the first contributor should:
1. Choose and document the technology stack (language, framework, database, etc.)
2. Initialize the project with a package manager (e.g., `npm init`, `pip init`, `cargo init`)
3. Set up the project directory structure
4. Add linting and formatting configuration
5. Set up a test framework
6. Create a `.env.example` for any environment variables
7. Update this file with the chosen stack and conventions

### Git Workflow

- **Default branch**: `main`
- Use feature branches with the naming pattern `claude/<description>-<session-id>`
- Write clear, descriptive commit messages
- Push with `git push -u origin <branch-name>`

### Conventions to Follow (once development begins)

- Keep code modular and well-organized
- Write tests for new features and bug fixes
- Do not commit secrets, credentials, or `.env` files
- Document API endpoints and database schemas as they are created
- Update this CLAUDE.md file as the project evolves

## Commands

_No build, test, or lint commands are configured yet. Update this section when they are._

```
# Placeholder — fill in as the project matures
# Build:    <not configured>
# Test:     <not configured>
# Lint:     <not configured>
# Start:    <not configured>
```

## Architecture

_No architecture has been defined yet. When the stack is chosen, document:_

- Backend framework and language
- Frontend framework (if applicable)
- Database and ORM/query layer
- Authentication strategy
- API design (REST, GraphQL, etc.)
- Deployment target

## Project Structure

```
Restaurant-Management/
├── README.md
├── CLAUDE.md
└── (no other files yet)
```

_Update this tree as directories and files are added._

## Key Decisions Log

Track important architectural and technical decisions here as they are made.

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-02-06 | Repository created | Initial project setup |

## Notes for AI Assistants

- This is a fresh repository. Do not assume any framework or tooling is present.
- Before writing code, confirm the intended technology stack with the user.
- When adding new tooling or dependencies, update this file accordingly.
- Keep the Commands section current so future sessions can run builds/tests without exploration.
- Prefer simple, well-structured solutions over complex abstractions, especially early in the project lifecycle.
