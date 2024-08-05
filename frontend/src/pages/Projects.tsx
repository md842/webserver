import './Projects.css'

import React from "react";

import { collection, getDocs } from "firebase/firestore";
import db from '../components/firebaseConfig.ts';

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

interface Project{
  /* Props interface for ProjectCard(). */
  // Unique key prop for each projectObj.                     Source: read().
  key: number;
  // Title of project.                                        Source: Database.
  title: string;
  // Description of project.                                  Source: Database.
  desc: string;
  // If present, card will display an image.                  Source: Database.
  image?: string;
  // Link to project repository.                              Source: Database.
  repo: string;
  // If present, "Run Simulation" button will appear.         Source: Database.
  sim?: string;
  // Tags associated with the project.                        Source: Database.
  tags: Array<string>;
}

function ProjectCard(params: Project): JSX.Element{
  /* Constructs a project card given Project object. */
  let unraveledTags = ""; // Convert tags array to string
  params.tags.forEach(element => unraveledTags += element + ", ");
  unraveledTags = unraveledTags.substring(0, unraveledTags.length - 2);
  return(
    <>
      <Card>
        {params.image && // Return img element if params.image is present
          <Card.Img variant="top" src={params.image}/>
        }
        <Card.Body>
          <Card.Title>{params.title}</Card.Title>
          <Card.Text>{params.desc}</Card.Text>
          <Card.Text>Tags: {unraveledTags}</Card.Text>
          {params.sim && // Return button element if params.sim is present
            <Button href={params.sim} variant="primary">
              Run Simulation
            </Button>
          }
          <Button href={params.repo} variant="primary">
            View repository on GitHub
          </Button>
        </Card.Body>
      </Card>
    </>
  );
}

export default class Projects extends React.Component<{}, FilterState>{
  featuredData: Array<Project>;
  projectData: Array<Project>;

  constructor(props: {}) {
    super(props);
    this.state = {filter: ""};
    this.featuredData = new Array<Project>;
    this.projectData = new Array<Project>;
    this.read(); // Read project data from database
  }

  handleChange(){
    console.log("Pending implementation!");
  }

  async read(){
    /* Read project data from database and populate lists for rendering. */
    let keyNum = 0; // Generate unique key props for each projectObj
    const dbQuery = await getDocs(collection(db, "projects"));
    dbQuery.forEach((doc) => {
      let projectObj:Project = {
        key: keyNum, // Unique key prop for each projectObj
        desc: doc.data().desc,
        title: doc.id,
        repo: doc.data().repo,
        image: doc.data().image,
        sim: doc.data().sim,
        tags: doc.data().tags
      };

      // Push completed projectObj to appropriate list
      if (doc.data().featured)
        this.featuredData.push(projectObj);
      else
        this.projectData.push(projectObj);

      keyNum++;
    });

    this.setState(this.state); // Update render after reading from database
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

          {this.featuredData.length == 0 &&
            <p>No matching projects were found!</p>
          }

          {this.featuredData.map((params:Project) => ( // From database
            <ProjectCard {...params}/>)) // Spread syntax to pass props object
          }

          <h3>Projects</h3>

          {this.projectData.length == 0 &&
            <p>No matching projects were found!</p>
          }

          {this.projectData.map((params:Project) => ( // From database
            <ProjectCard {...params}/>)) // Spread syntax to pass props object
          }
        </main>
      </>
    );
  }
}