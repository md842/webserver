import './Projects.css'

import React from 'react';

import Button from 'react-bootstrap/Button';
import Card from 'react-bootstrap/Card';
import Container from 'react-bootstrap/Container';
import Form from 'react-bootstrap/Form';
import InputGroup from 'react-bootstrap/InputGroup';
import ToggleButton from 'react-bootstrap/ToggleButton';
import ToggleButtonGroup from 'react-bootstrap/ToggleButtonGroup';

interface FilterState{
  filter: string;
}

interface CardParams{
  description: string;
  title: string;
  repoLink: string;
  image?: string; /* If present, card will display an image */
  simLink?: string; /* If present, "Run Simulation" button will appear */
  tags: Array<string>
  filter: string;
}

function ProjectCard(params: CardParams): JSX.Element{
  /* Constructs a project card given CardParams */
    let filterMatch = (params.tags.indexOf(params.filter) > -1) || (params.filter === "");
    let unraveledTags = "";
    params.tags.forEach(element => unraveledTags += element + ", ");
    unraveledTags = unraveledTags.substring(0, unraveledTags.length - 2);
    if (filterMatch){
      return(
        <>
          <Card>
            { /* Only return img element if params.image is present */
            params.image &&
              <Card.Img variant="top" src={params.image}/>
            }
            <Card.Body>
              <Card.Title>{params.title}</Card.Title>
              <Card.Text>{params.description}</Card.Text>
              <Card.Text>Tags: {unraveledTags}</Card.Text>
              { /* Only return button element if params.simLink is present */
              params.simLink &&
                <Button
                  href={params.simLink}
                  variant="primary"
                >
                  Run Simulation
                </Button>
              }
              <Button
                href={params.repoLink}
                variant="primary"
              >
                View repository on GitHub
              </Button>
            </Card.Body>
          </Card>
        </>
      );
    }
    else{
      return(
        <></>
      );
    }
  }

export default class Projects extends React.Component<{}, FilterState>{
  constructor(props: {}) {
    super(props);
    this.state = {filter: ""};
  }

  handleChange(){
    console.log("Pending implementation!");
  }

  render(){
    return(
      <>
        <main>
          <InputGroup className="mb-3">
            <Form.Control
              placeholder="Search for projects... (Pending implementation)"
            />
            <Button id="search-btn">Search</Button>
          </InputGroup>

          <Container fluid className="mb-5" id="filter-container">
            <p id="filter-label">Or filter by tag: (Pending implementation)</p>
            <ToggleButtonGroup type="checkbox" className="filter-btns" onChange={this.handleChange}>
              <ToggleButton id="filter-js" value={1}>JavaScript</ToggleButton>
              <ToggleButton id="filter-opengl" value={2}>OpenGL</ToggleButton>
              <ToggleButton id="filter-cpp" value={3}>C++</ToggleButton>
            </ToggleButtonGroup>
          </Container>

          <h3>Featured Projects</h3>

          <ProjectCard
            description="Placeholder description text"
            title="Earth Impact Simulator"
            repoLink="https://github.com/md842/earth-impact-simulator"
            image="simulations/earth-impact-simulator/thumb.png"
            simLink="projects/earth-impact-simulator"
            tags={["WebGL", "OpenGL", "OpenGL Shading Language (GLSL)", "JavaScript"]}
            filter={this.state.filter}
          />

          <br/>

          <h3>Projects</h3>

          <ProjectCard
            description="Placeholder description text"
            title="Placeholder Title"
            repoLink=""
            tags={["C++"]}
            filter={this.state.filter}
          />
        </main>
      </>
    );
  }
}