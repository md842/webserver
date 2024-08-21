import './CPPTest.css'

import React from "react";

import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';

interface IOState{
  input: string;
  output: string;
}

export default class CPPTest extends React.Component<{}, IOState>{
  constructor(props: {}) {
    super(props);
    this.state = {input: "147\n0\n16\n0\n19\n1\n128\n0\n179\n1\n17\n64\n51\n66\n17\n0\n179\n82\n17\n64\n51\n3\n50\n0\n147\n115\n50\n0\n51\n133\n67\n64\n179\n5\n51\n0", output: ""};
  }

  submit(){
    fetch('/', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: JSON.stringify({
        "cpp_input": this.state.input
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

  handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({input: e.target.value}); // Set input state to changed text
  }

  render(){
    return(
      <>
        <main>
            <h1>C++ Backend Test</h1>
            <div className="description">
              <p>
                Placeholder description
              </p>
              <p>
                Tags: C++
              </p>
              <Button
                variant="primary"
                href="/projects"
              >
                Back to projects
              </Button>
            </div>
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
            </Form>

            <Form.Label>Output</Form.Label>
            <Form.Control
              as="textarea"
              rows={10}
              value={this.state.output}
              readOnly
            />
        </main>
      </>
    );
  }
}