import './NotebookViewer.css'

import React from 'react';

import { doc, getDoc } from "firebase/firestore";
import db from '../../components/firebaseConfig.ts';

import Button from 'react-bootstrap/Button';

interface Notebook{
  // Title of project.                                        Source: Database
  title: string;
  // Long form description of project for page display.       Source: Database
  long_desc: string;
  // Location of notebook to embed.                           Source: Database
  nb_embed: string;
  // Tags associated with the project.                        Source: Database
  tags: string;
}

export default class NotebookViewer extends React.Component<{}, Notebook>{
  constructor(props: {}) {
    super(props);
    this.state = {
      long_desc: "Loading from database...",
      nb_embed: "",
      tags: "Loading from database...",
      title: "Loading from database..."
    };

    const set_dyn_height = () => {
      document.documentElement.style.setProperty('--dyn-height',
        "calc(" + window.innerHeight * 0.9 + "px - 70px - 5em");
    }
    set_dyn_height(); // Set --dyn-height immediately one time
    window.addEventListener("resize", set_dyn_height); // Add resize listener

    this.read(); // Read project data and tags from database
  }

  async read(){
    // Remove "/projects/notebooks/" (length 20) from pathname to get target id
    let target_id = window.location.pathname.substring(20);
    const data = (await getDoc(doc(db, "projects", target_id))).data();

    let unraveledTags = ""; // Convert tags array to string
    data!.tags.forEach((element: string) => unraveledTags += element + ", ");
    unraveledTags = unraveledTags.substring(0, unraveledTags.length - 2);

    this.setState({
      long_desc: data!.long_desc,
      nb_embed: data!.nb_embed,
      tags: unraveledTags,
      title: data!.title
    });
  }
  
  render(){
    return(
      <>
        <main>
          <div className="dyn-div">
            <h1>{this.state.title}</h1>
            <div className="description">
              <p>{this.state.long_desc}</p>
              <p>Tags: {this.state.tags}</p>
              <Button
                variant="primary"
                href="/projects"
              >
                Back to projects
              </Button>
            </div>
            <embed src={this.state.nb_embed}/>
          </div>
        </main>
      </>
    );
  }
}