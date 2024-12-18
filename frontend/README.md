# Max's Personal Website
This repository contains the source code and documentation for my personal website.

All console commands given in this document are for Arch Linux kernel 6.9.6-arch1-1 and assume you have already cloned the top level repository.

This project was bootstrapped with [Vite](https://vitejs.dev/).

## Dependencies (required)

#### Node.js (nodejs-22.5.0-1 used)
```console
$ sudo pacman -S npm
```

#### Earth Impact Simulator
```console
$ cd src/assets
$ git clone git@github.com:md842/earth-impact-simulator.git
```

## How to Build
1. Install required dependencies from previous section.
2. Navigate to the `frontend/` directory.
```console
$ cd webserver
webserver$ cd frontend
```

3. Install package dependencies.
```console
frontend$ npm install
```

4. Run the production build.
```console
frontend$ npm run build
```

## How to Run
To run the app in development mode:
```console
frontend$ npm run dev
```

The app can now be accessed at `http://localhost:5173/`.