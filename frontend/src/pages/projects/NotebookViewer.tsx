import './NotebookViewer.css'

import React from 'react';

import {doc, getDoc} from "firebase/firestore";
import db from '../../components/firebaseConfig.ts';
import NavButton from '../../components/NavButton.tsx';

import Button from 'react-bootstrap/Button';

interface Notebook{
  // Long form description of project for page display.       Source: Database
  long_desc: string;
  // Location of notebook to embed.                           Source: Database
  nb_embed: string;
  // Link to project repository.                              Source: Database
  repo?: string;
  // Tags associated with the project.                        Source: Database
  tags: string;
  // Title of project.                                        Source: Database
  title: string;
}

export default class NotebookViewer extends React.Component<{}, Notebook>{
  constructor(props: {}){
    super(props);
    this.state = {
      long_desc: "Loading from database...",
      nb_embed: "",
      tags: "Loading from database...",
      title: "Loading from database..."
    };

    const set_dyn_height = () => {
      document.documentElement.style.setProperty('--dyn-height',
        "calc(" + window.innerHeight + "px - 1em)");
    }
    set_dyn_height(); // Set --dyn-height immediately one time
    window.addEventListener("resize", set_dyn_height); // Add resize listener

    this.read(); // Read project data and tags from database
  }

  async read(){
    // Remove "/projects/notebooks/" (length 20) from pathname for target id
    let target_id = window.location.pathname.substring(20);
    const data = (await getDoc(doc(db, "projects", target_id))).data();

    let uDesc = ""; // Unravel long_desc array to string
    data!.long_desc.forEach((element: string) => uDesc += element + '\n\n');

    let uTags = ""; // Unravel tags array to string
    data!.tags.forEach((element: string) => uTags += element + ", ");
    uTags = uTags.substring(0, uTags.length - 2); // Remove the last comma

    this.setState({
      long_desc: uDesc,
      // Use naming convention of notebook files to get embed
      nb_embed: "/notebooks/" + target_id + "-nb.html",
      repo: data?.repo,
      tags: uTags,
      title: data!.title
    });
  }
  
  render(){
    return(
      <main>
        <h1>{this.state.title}</h1>
        <div className="description">
          <p className="long-desc">{this.state.long_desc}</p>
          <p>Tags: {this.state.tags}</p>
          <NavButton href="/projects">Back to projects</NavButton>
          {(this.state.repo) && // Render only if repo is set
            <Button onClick={() => window.open(this.state.repo)}>
              View repository on GitHub
            </Button>
          }
        </div>
        <div className="nb-container">
          <iframe src={this.state.nb_embed}/>
        </div>
      </main>
    );
  }
}