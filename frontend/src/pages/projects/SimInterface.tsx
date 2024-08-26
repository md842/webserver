import './SimInterface.css';

import React from "react";

import {doc, getDoc} from "firebase/firestore";
import db from '../../components/firebaseConfig.ts';
import NavButton from '../../components/NavButton.tsx';

import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';

interface IOState{
  // Input to the simulation.                                 Source: User/DB
  input: string;
  // Output of the simulation.                                Source: Server
  output: string;
  // Indicates whether the simulation input is raw or file.   Source: Database
  input_as_file: boolean;
  // Title of project.                                        Source: Database
  title: string;
  // Long form description of project for page display.       Source: Database
  long_desc: string;
  // Link to project repository.                              Source: Database
  repo: string;
  // Tags associated with the project.                        Source: Database
  tags: string;
}

export default class SimInterface extends React.Component<{}, IOState>{
  constructor(props: {}) {
    super(props);
    this.state = {
      input: "Loading from database...",
      output: "",
      input_as_file: false,
      long_desc: "Loading from database...",
      repo: "",
      tags: "Loading from database...",
      title: "Loading from database..."
    };

    this.read(); // Read project data and tags from database
  }

  handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({input: e.target.value}); // Set input state to changed text
  }

  async read(){
    // Remove "/projects/sim/" (length 14) from pathname for target id
    let target_id = window.location.pathname.substring(14);
    const data = (await getDoc(doc(db, "projects", target_id))).data();

    let unraveledTags = ""; // Convert tags array to string
    data!.tags.forEach((element: string) => unraveledTags += element + ", ");
    unraveledTags = unraveledTags.substring(0, unraveledTags.length - 2);

    this.setState({
      // Firestore stores \n as literal "\\n", so replace with newlines \n
      input: data!.default_input.replaceAll("\\n", '\n'),
      input_as_file: data!.input_as_file,
      long_desc: data!.long_desc,
      repo: data!.repo,
      tags: unraveledTags,
      title: data!.title
    });
  }

  submit(){
    fetch('/', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: JSON.stringify({
        "input": this.state.input,
        "input_as_file": this.state.input_as_file,
        "source": window.location.pathname.substring(14),
      })
    })
    .then(response => response.json())
    .then((data) => {
      console.log(data);
      // Set output state to response data
      this.setState({output: data.output});
    })
    .catch((error) => {
      this.setState({output: error}); // Set output state to caught error
    });
  }

  render(){
    return(
      <main>
        <h1 className="mb-4">{this.state.title}</h1>
        <div className="description">
          <p>{this.state.long_desc}</p>
          <p>Tags: {this.state.tags}</p>
          <p>
            This simulation runs on a custom interface that communicates with
            the back-end via a POST request, using JSON encoded data to specify
            an executable to run server-side and provide user-customizable
            input for the executable. The web server runs the executable as a
            background process, piping its output and encoding it as JSON
            within an HTTP response. Security mechanisms are in place to
            protect the server from excessive and/or unintended payloads.
          </p>
          <NavButton href="/projects">Back to projects</NavButton>
          <Button href={this.state.repo}>View repository on GitHub</Button>
        </div>
        <div className="backend-container">
          <p className="bg-dark">C++ Back-end Interface</p>
          <Form>
            <Form.Group className="mb-3" controlId="cpp-input">
              <Form.Label>Input</Form.Label>
              <Form.Control
                as="textarea"
                className="mb-3"
                rows={10}
                onChange={(e) => this.handleChange(e as any)}
                value={this.state.input}
              />
              <Button onClick={() => {this.submit()}}>Submit</Button>
            </Form.Group>
            <Form.Label>Output</Form.Label>
            <Form.Control
              as="textarea"
              rows={10}
              value={this.state.output}
              readOnly
            />
          </Form>
        </div>
      </main>
    );
  }
}