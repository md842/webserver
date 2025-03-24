import './ProjectCard.css'

import {useState} from "react";

import {collection, getDocs} from "firebase/firestore";
import db from '../components/firebaseConfig.ts';
import NavButton from '../components/NavButton.tsx';

import Button from 'react-bootstrap/Button';
import Card from 'react-bootstrap/Card';

/** Props interface for ProjectCard(). */
export interface Project{
  // Title of project.                                        Source: Database
  title: string;
  // Short form description of project for card display.      Source: Database
  card_desc: string;
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

/** Constructs a Card element given a Project object specified by params. */
export function ProjectCard(params: Project): JSX.Element{
  const [visible, setVisible] = useState(params.vis);
  if (params.vis != visible) // visible state is out of date
    setVisible(params.vis); // Update visible state

  let unraveledTags = ""; // Convert tags array to string
  params.tags.forEach(element => unraveledTags += element + ", ");
  unraveledTags = unraveledTags.substring(0, unraveledTags.length - 2);

  return(
    <Card className={visible ? "card-in" : "card-out"}>
      {params.image && // Return img element if params.image is present
        <Card.Img
          className={visible ? "img-in" : "img-out"} // CSS animations
          src={params.image}
        />
      }
      <Card.Body className={visible ? "body-in" : "body-out"}>
      <Card.Title>{params.title}</Card.Title>
      <Card.Text>{params.card_desc}</Card.Text>
      <Card.Text>Tags: {unraveledTags}</Card.Text>
      {params.sim && // Return NavButton element if params.sim is present
        <NavButton href={params.sim}>Run Simulation</NavButton>
      }
      {params.nb && // Return NavButton element if params.nb is present
        <NavButton href={params.nb}>View Notebook</NavButton>
      }
      {params.repo && // Return Button element if params.repo is present
        <Button onClick={() => window.open(params.repo)}>
          View repository on GitHub
        </Button>
      }
      </Card.Body>
    </Card>
  );
}

/** 
 * Reads project data from the database with two modes, all or featured only.
 * @param all If true, reads both featured and non-featured projects.
 *     If false, reads featured projects only.
 * @return If all = true, returns two arrays of Project objects where element 0
 *     contains featured projects and element 1 contains non-featured projects.
 *     If all = false, returns one array containing only featured projects.
 */
export async function readProjectData(all: boolean): Promise<Array<Array<Project>>>{
  let out = [new Array<Project>]; // Initialize Array<Array<Project>> output
  if (all) // Only allocate the extra memory if it will be used
    out.push(new Array<Project>);

  const dbQuery = await getDocs(collection(db, "projects"));
  dbQuery.forEach((doc) => { // For each document in the "projects" collection:
    if (all || doc.data().featured){ // Only create object if it will be used
      let projectObj: Project = {
        card_desc: doc.data().card_desc,
        title: doc.data().title,
        repo: doc.data().repo,
        image: doc.data().image,
        nb: doc.data().nb,
        sim: doc.data().sim,
        tags: doc.data().tags,
        vis: true // Initial visbility is always true
      };
      if (doc.data().featured) // Featured, push to element 0 of output
        out[0].push(projectObj);
      else // Non-featured, all = true, push to element 1 of output
        out[1].push(projectObj);
    }
  });
  return out; // async function implicitly wraps output in a Promise
}