#!/bin/bash
# CBuild Project Scaffold Script

if [ -z "$1" ]; then
    echo "Usage: ./new_project.sh <ProjectName>"
    exit 1
fi

PROJECT_NAME=$1

mkdir -p "$PROJECT_NAME"
cd "$PROJECT_NAME"

mkdir -p assets/fonts assets/images backend
cat <<EOF > "$PROJECT_NAME.cbuild"
project_name=$PROJECT_NAME
ollama_model=llama3
ollama_host=localhost
EOF

cat <<EOF > backend/server.js
// Custom Backend stub
const http = require('http');

const server = http.createServer((req, res) => {
  if (req.url === '/health') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ status: 'ok' }));
  } else {
    res.writeHead(404);
    res.end();
  }
});

server.listen(3000, () => {
  console.log('Custom Backend running on port 3000');
});
EOF

echo "Project $PROJECT_NAME scaffolded successfully."
