# Personal Web Server: Product Requirements Document
Author: Max Deng  
Last Updated: June 27, 2024

### Vision
A custom web server built from scratch for personal use. 

### Motivation
I would like to host a website to display my personal portfolio. I decided to eschew the use of website templates and turn the website into its own project. The objective is to demonstrate my experience with full stack web development and software testing in a unique way.

### Goals
- Build a web server from scratch capable of serving a web page. 
- Build a modern personal website for the webserver to serve.
- Host the webserver such that it is accessible on the Internet.

### Non-Goals
- The web server is not mission-critical and high traffic is not expected, so a distributed system architecture is not required.
- The web server need not adhere to the full configuration file spec, only what is required by the feature scope of the web server.

### Requirements

#### Web Server:
- **HTTP Request Handling:** The web server shall respond to HTTP requests, including malformed requests, appropriately.
- **Concurrency:** The web server shall be able to handle concurrent requests without deadlock or correctness issues.
- **File Serving:** The web server shall be able to serve multiple web pages with different URIs and file locations. In addition, the web server shall be able to host other file types, such as images. 
- **Logging:** The web server will keep detailed, machine-parseable logs of requests received, response statuses, and errors. Additionally, the machine may keep trace logs for debugging.
- **Configurability:** The web server shall be configurable in adherence with a standard configuration file format.

#### Web Page:
- **Technological Modernity:** The webpage shall use modern technologies and frameworks to achieve a responsive, high-quality look and feel.
- **Mobile Compatibility:** The webpage shall adapt to viewing on mobile devices by rearranging elements to suit a vertical screen layout.

### Success Criteria

#### Quantitative Measures:
- **Responsiveness:** No more than 200ms shall elapse between receiving an HTTP request and displaying a rendered webpage on the client's browser.
- **Testing Coverage:** Unit tests and integration tests shall achieve a minimum of 90% coverage on the code base.

#### Qualitative Measures:
- **Polish:** The webpage delivered shall look and feel professionally made.