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
  cerr: string;
  // Output of the simulation.                                Source: Server
  cout: string;
  cerr_size: number;
  cout_size: number;
  cerr_name: string;
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
      cerr: "",
      cout: "",
      cerr_size: 10,
      cout_size: 3,
      cerr_name: "",
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

    let uDesc = ""; // Unravel long_desc array to string
    data!.long_desc.forEach((element: string) => uDesc += element + '\n\n');

    let uTags = ""; // Unravel tags array to string
    data!.tags.forEach((element: string) => uTags += element + ", ");
    uTags = uTags.substring(0, uTags.length - 2); // Remove the last comma

    this.setState({
      // Firestore stores \n as literal "\\n", so replace with newlines \n
      input: data!.default_input.replaceAll("\\n", '\n'),
      cout_size: data!.cout_size,
      cerr_size: data!.cerr_size,
      cerr_name: data!.cerr_name,
      input_as_file: data!.input_as_file,
      long_desc: uDesc,
      repo: data!.repo,
      tags: uTags,
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
      this.setState({ // Set output states to response data
        cout: data.cout,
        cerr: data.cerr,
      });
    })
    .catch((error) => {
      this.setState({cout: error}); // Set output state to caught error
    });
  }

  render(){
    return(
      <main>
        <h1 className="mb-4">{this.state.title}</h1>
        <div className="description">
          <p className="long-desc">{this.state.long_desc}</p>
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
              rows={this.state.cout_size}
              value={this.state.cout}
              readOnly
            />
            {(this.state.cerr_name != "") && // Display if cerr_name is set
              <div className="cerr">
                <Form.Label>{this.state.cerr_name}</Form.Label>
                <Form.Control
                  as="textarea"
                  rows={this.state.cerr_size}
                  value={this.state.cerr}
                  readOnly
                />
              </div>
            }
          </Form>
        </div>
      </main>
    );
  }
}