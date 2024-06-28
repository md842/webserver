# Personal Web Server: Technical Design Proposal
Author: Max Deng  
Last Updated: June 27, 2024

### Objective
I currently lack a personal website on which to display my projects. Therefore, I am creating a personal website and implementing a web server that will host it. The web server will support the HTTP/1.1 protocol and be able to serve web pages (and other files) as I wish.

### Requirements
The web server portion of this project shall handle requests according to HTTP/1.1 specifications and be able to serve files of various types from configurable directories. The configuration file shall follow the NGINX configuration file format. It shall keep detailed machine-parsable logs and handle concurrent requests using threading. It shall not implement the full NGINX configuration file specifications, and it shall not require a distributed system architecture.

The web page portion of this project shall be responsive, adaptive to mobile devices, and look modern and professional.

### Detailed Design

#### Web Server:

The web server will be written in C++ and use Boost C++ libraries. 

- **HTTP Request Handling:** The web server shall parse incoming requests using the `boost::beast::http::request` format. The server shall recognize correct HTTP requests and provide the expected HTTP/1.1 response. Additionally, the web server shall recognize malformed HTTP requests and respond with appropriate error codes. Responses will follow the `boost::beast::http::response` format.
- **Concurrency:** The web server shall start a new concurrent execution thread for each HTTP request it receives, and use dependency injection to dynamically dispatch a short-lived handler per request. This enables concurrent request handling without fear of deadlock. There are no plans to support client-side write operations (such as `POST`, `PUT`, or `DELETE`) at this time, negating the concern of correctness issues.
- **File Serving:** The web server shall be able to serve multiple web pages with different URIs and file locations. In addition, the web server shall be able to host other file types, such as images. 
- **Logging:** The web server will use the `Boost::log` library to generate detailed, machine-parseable logs of requests received, response statuses, and errors. Additionally, the machine may keep trace logs for debugging.
- **Configurability:** The web server's serving port and file serving directories shall be configurable in adherence with a subset of the NGINX configuration file format. The full NGINX spec need not be supported. In the case that a request URI matches multiple file serving directories within the configuration file, the deepest match will take precedence. Example configuration file below:
```
port 80;

location / FileRequestHandler {
  root /servable_files;
}

location /docs FileRequestHandler {
  root /docs;
}
```

#### Web Page:

The web page will be written in React/JSX and CSS 3.

- **Technological Modernity:** The web page shall use React to deliver dynamic content in a way that feels responsive to the user. For example, using `BrowserRouter` can avoid re-rendering content that is common between different pages, improving performance. Modern CSS frameworks, such as Bootstrap, will be utilitized to give the web page a modern and professional appearance.
- **Mobile Compatibility:** The webpage shall adapt to viewing on mobile devices by rearranging elements to suit a vertical screen layout. This will be implemented within the CSS.

### Alternatives Considered
- I considered using a long-lived request handler object rather than a dynamically dispatched per-request object for ease of programming. However, this causes issues with concurrency, as the object may only handle one request at a time.
- Using a static HTML 5 webpage was considered for simplified file serving, but I ultimately decided that the technological benefits of React are worth the extra work.