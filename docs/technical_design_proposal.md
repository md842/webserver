# Personal Web Server: Technical Design Proposal
Author: Max Deng  
Last Updated: January 5, 2026

### Objective
I am implementing a custom web server that will host (but not necessarily be tied to) my personal website.

### Requirements
The web server shall handle requests according to HTTP/1.1 specifications and be able to serve files of various types from configurable directories. It will additionally support the TLS protocol and be able to process HTTPS requests. The configuration file shall follow a subset of the Nginx configuration file format. It shall keep detailed machine-parsable logs and handle concurrent requests using threading.

The web server shall not implement the full Nginx configuration file specifications, and it shall not require a distributed system architecture.

### Detailed Design
The web server will be implemented in C++ with Boost C++ libraries.

- **HTTP/HTTPS Request Handling:** The web server shall parse incoming requests using the `boost::beast::http::request` format. It shall recognize valid HTTP/HTTPS requests and provide an appropriate HTTP/1.1 response. Additionally, the web server shall recognize malformed requests and respond with appropriate error codes. Responses will follow the `boost::beast::http::response` format.
- **Concurrency:** The web server shall start a new concurrent execution thread for each request it receives, and use dependency injection to dynamically dispatch a short-lived handler per request. This enables concurrent request handling without fear of deadlock. There are no plans to support client-side write operations (such as `POST`, `PUT`, or `DELETE`) at this time, negating the concern of correctness issues.
- **File Serving:** The web server shall be able to serve multiple web pages with different URIs and file locations. In addition, the web server shall be able to host other file types, such as images. 
- **Logging:** The web server will use the `Boost::log` library to generate detailed, machine-parseable logs of requests received, response statuses, and errors. Additionally, the machine may keep trace logs for debugging.
- **Configurability:** The web server shall be configurable in adherence with a subset of the Nginx configuration file format. The full Nginx spec need not be supported. In the case that a request URI matches multiple file serving directories within the configuration file, the deepest match will take precedence. 

  - The web server implements the following Nginx directives: `http`, `server`, `location`, `listen`, `index`, `root`, `server_name`, `ssl_certificate`, `ssl_certificate_key`, `try_files`, and `return`.
  - The web server implements the following configuration variables: `$host` and `$scheme` within the context of a `return` directive, and `$uri` within the context of a `try_files` directive.
  - The web server implements the following location modifiers: `=` (exact match), `^~` (longest prefix match with stop modifier), and no modifier (longest prefix match). **Note:** Because regex modifiers `~` and `~*` are not implemented, `^~` is functionally identical to no modifier.

  Example configuration file below:

  ```
  http {
    server { # HTTPS server using production frontend
      listen                8080 ssl;
      index                 index.html;
      root                  ../personal-website/build;

      server_name           localhost;

      ssl_certificate        tests/certs/localhost.crt;
      ssl_certificate_key    tests/certs/localhost.key;

      location = / { # Check for exact match
        # React Router path; serve index
      }

      location ^~ /projects/sim { # Longest prefix match
        try_files index.html =404; # React Router path; serve index
      }

      location / { # Fallback
        try_files $uri index.html; # React Router handles 404 by serving index
      }
    }
    server { # Redirects from HTTP to HTTPS (expected result: 301 Moved Permanently)
      listen                8081;
      server_name           localhost;
      return                301 https://$host:8080$request_uri;
    }

    server { # Redirects from HTTP to HTTP via $scheme (expected result: 302 Found)
      listen                8082;
      server_name           localhost;
      return                302 $scheme://$host:8081$request_uri;
    }
  }
  ```

### Alternatives Considered
- I considered using a long-lived request handler object rather than a dynamically dispatched per-request object for ease of programming. However, this causes issues with concurrency, as the object may only handle one request at a time.
- Using a static HTML 5 webpage was considered for simplified file serving, but I ultimately decided that the technological benefits of React are worth the extra work.