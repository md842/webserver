# Personal Web Server: Product Requirements Document
Author: Max Deng  
Last Updated: January 5, 2026

### Vision
A custom high performance web server built from scratch for my personal use.

### Motivation
I would like to host a website to showcase my personal portfolio. I decided to eschew the use of available website templates and web server software and turn my personal website into a full stack project. The objective is to demonstrate my experience with full stack development and software testing in a unique way.

### Goals
- Build a web server capable of serving a React Router based web page as well as static files.
- Host the web server such that it is publicly accessible over the Internet.

### Non-Goals
- The web server is not mission-critical and high traffic is not expected, so a distributed system architecture is not required.
- The web server need not adhere to the full configuration file spec, only what is required by the feature scope of the web server.

### Requirements
- **HTTP/HTTPS Request Handling:** The web server shall respond appropriately to HTTP and HTTPS requests, including malformed requests.
- **Concurrency:** The web server shall be able to handle concurrent requests without deadlock or correctness issues.
- **File Serving:** The web server shall be able to serve multiple web pages with different URIs and file locations. In addition, the web server shall be able to host other file types, such as images. 
- **Logging:** The web server will keep detailed, machine-parseable logs of requests received, response statuses, and errors. Additionally, the machine may keep trace logs for debugging.
- **Configurability:** The web server shall be configurable in adherence with a standard configuration file format.

### Success Criteria

#### Quantitative Measures:
- **Reliability:** The web server shall be highly available, with uptime exceeding 99.99%.
- **Responsiveness:** The time between receiving a request and sending an appropriate response shall not exceed 25ms.
- **Testing Coverage:** Unit tests and integration tests shall achieve a minimum of 90% coverage on the code base.

#### Qualitative Measures:
- **Polish:** The web server source code shall be well documented and easy to maintain.