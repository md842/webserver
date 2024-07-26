# Max's Personal Web Server
This repository contains the source code and documentation for my personal website.

All console commands given in this document assume Arch Linux kernel 6.9.6-arch1-1.

This project was bootstrapped with [Vite](https://vitejs.dev/).

## Dependencies (required)

#### Node.js (nodejs-22.5.0-1 used)
```console
$ sudo pacman -S npm
```

## How to Build
1. Install required dependencies from previous section.
2. Clone this repository.
```console
$ git clone https://github.com/md842/webserver
```
3. Navigate to the project directory.
```console
$ cd webserver
webserver$ cd frontend
```

4. Install package dependencies.
```console
frontend$ npm install
```

5. Run the production build.
```console
frontend$ npm run build
```

## How to Run
To run the app in development mode:
```console
frontend$ npm run dev
```

The app can now be accessed at `http://localhost:5173/`.