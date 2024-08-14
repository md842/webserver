import './Projects.css'

import React, { useState } from "react";

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
  filter: Record<string, boolean>;
}

interface Project{
  /* Props interface for ProjectCard(). */
  // Title of project.                                        Source: Database
  title: string;
  // Description of project.                                  Source: Database
  desc: string;
  // If present, card will display an image.                  Source: Database
  image?: string;
  // If present, "View Notebook" button will appear.          Source: Database
  nb?: string;
  // Link to project repository.                              Source: Database
  repo?: string;
  // If present, "Run Simulation" button will appear.         Source: Database
  sim?: string;
  // Tags associated with the project.                        Source: Database
  tags: Array<string>;
  // The visibility of this card.                             Source: Section()
  vis: boolean;
}

interface SectionParams{
  title: string;
  data: Array<Project>;
}

function ProjectCard(params: Project): JSX.Element{
  /* Constructs a project card given Project object specified by params. */
  const [visible, setVisible] = useState(params.vis);
  if (params.vis != visible) // visible state is out of date
    setVisible(params.vis); // Update visible state

  let unraveledTags = ""; // Convert tags array to string
  params.tags.forEach(element => unraveledTags += element + ", ");
  unraveledTags = unraveledTags.substring(0, unraveledTags.length - 2);

  return(
    <>
      <Card className={visible ? "card-in" : "card-out"}>
        {params.image && // Return img element if params.image is present
          <Card.Img
            className={visible ? "img-in" : "img-out"} // CSS animations
            variant="top"
            src={params.image}
          />
        }
        <Card.Body className={visible ? "body-in" : "body-out"}>
        <Card.Title>{params.title}</Card.Title>
        <Card.Text>{params.desc}</Card.Text>
        <Card.Text>Tags: {unraveledTags}</Card.Text>
        {params.sim && // Return button element if params.sim is present
          <Button href={params.sim} variant="primary">Run Simulation</Button>
        }
        {params.nb && // Return button element if params.nb is present
          <Button href={params.nb} variant="primary">View Notebook</Button>
        }
        {params.repo && // Return button element if params.nb is present
          <Button href={params.repo} variant="primary">
            View repository on GitHub
          </Button>
        }
        </Card.Body>
      </Card>
    </>
  );
}

export default class Projects extends React.Component<{}, FilterState>{
  featuredData: Array<Project>;
  projectData: Array<Project>;
  tags: Array<string>;

  constructor(props: {}) {
    super(props);
    this.state = {filter: {}};

    this.featuredData = new Array<Project>;
    this.projectData = new Array<Project>;
    this.tags = new Array<string>;

    this.read(); // Read project data and tags from database
  }

  FilterButtons = (): JSX.Element => {
    /* Renders the filter buttons at the top of the page using database tags.
       Class member due to use of this.handleChange. */
    const [checked] = useState(false);
    return(
      <>
        <Container fluid className="mb-5" id="filter-container">
          <p id="filter-label">Or filter by tag:</p>
          <ToggleButtonGroup type="checkbox" className="filter-btns">
            { /* Generate buttons from database tags. Key warnings can be
                 safely ignored; this map won't change after being rendered. */
            this.tags.map((tag:string) => (
              <ToggleButton
                id={"filter-" + tag}
                type="checkbox"
                variant="primary"
                checked={checked}
                value={tag}
                onChange={(e) => {this.handleChange(e)}}
              >
                {tag}
              </ToggleButton>))
            }
          </ToggleButtonGroup>
        </Container>
      </>
    );
  }

  handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    /* Updates this.state.filter based on the states of the filter buttons. */
    // Use button name as a key, set boolean to checked state
    console.log("Clicked");
    this.state.filter[e.currentTarget.value] = e.currentTarget.checked;
    this.setState(this.state); // Update render after updating filter
  }

  Section = (params: SectionParams): JSX.Element => {
    /* Renders a project section, or appropriate message if none rendered.
       Class member due to use of this.resolveFilter. */
    let displayed = false;
    return(
      <div className="projects-section mb-5">
        <h3 className="mb-3">{params.title}</h3>
        { // If data is not yet loaded from database:
        this.projectData.length == 0 &&
          <p>Loading projects from database...</p>
        }

        {/* Generate project cards from database info. Key warnings can be
            safely ignored; this map won't change after being rendered. */
          params.data.map((params:Project) => {
            params.vis = this.resolveFilter(params.tags); // Update vis state
            if (params.vis) // If any card is displayed in this section
              displayed = true; // Prevents message below from rendering
            return <ProjectCard {...params}/>; // Re-render with updated vis
          })
        }

        { // If data is loaded from database but no cards were displayed:
        (this.projectData.length > 0 && !displayed) &&
          <p>No projects were found that matched the filter settings.</p>
        }
      </div>
    );
  }

  async read(){
    /* Read from database and populate lists for rendering. */
    const tagsQuery = await getDocs(collection(db, "tags"));
    tagsQuery.forEach((doc) => {
      this.tags.push(doc.id)
    });

    const projectsQuery = await getDocs(collection(db, "projects"));
    projectsQuery.forEach((doc) => {
      let projectObj:Project = {
        desc: doc.data().desc,
        title: doc.id,
        repo: doc.data().repo,
        image: doc.data().image,
        nb: doc.data().nb,
        sim: doc.data().sim,
        tags: doc.data().tags,
        vis: true
      };

      // Push completed projectObj to appropriate list
      if (doc.data().featured)
        this.featuredData.push(projectObj);
      else
        this.projectData.push(projectObj);
    });

    this.setState(this.state); // Update render after reading from database
  }

  resolveFilter(tags: Array<string>): boolean{
    /* Helper function to determine whether a ProjectCard should be shown. */
    if (Object.keys(this.state.filter).length == 0)
      return true; // Trivial case: If the filter is empty, always display card

    let filtersDisabled = true;
    for (const key in this.state.filter){
      /* Non-trivial case: If at least one filter is enabled, display card only
         if a tag matches */
      if (this.state.filter[key] == true){
        filtersDisabled = false; // At least one filter is enabled
        // Search for the key inside the tag (partial match allowed)
        if (tags.some((element:string) => element.indexOf(key) != -1))
          return true;
      }
    }
    return filtersDisabled; // Trivial case: All disabled, always display card
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

          {/* Must be its own function to use "checked" hook */}
          <this.FilterButtons/>

          {/* Project sections. Data passed as reference, no memory waste. */}
          <div className="sections">
            <this.Section title="Featured Projects" data={this.featuredData}/>
            <this.Section title="Projects" data={this.projectData}/>
          </div>
        </main>
      </>
    );
  }
}