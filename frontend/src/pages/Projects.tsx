import './Projects.css'

import React, {useState} from "react";

import {collection, getDocs} from "firebase/firestore";
import db from '../components/firebaseConfig.ts';
import {Project, ProjectCard, readProjectData} from '../components/ProjectCard.tsx';

import Container from 'react-bootstrap/Container';
import Form from 'react-bootstrap/Form';
import ToggleButton from 'react-bootstrap/ToggleButton';
import ToggleButtonGroup from 'react-bootstrap/ToggleButtonGroup';

interface FilterState{
  filter: Record<string, boolean>;
  search: string;
}

interface SectionParams{
  title: string;
  data: Array<Project>;
}

export default class Projects extends React.Component<{}, FilterState>{
  featuredData: Array<Project>;
  projectData: Array<Project>;
  filterButtonTags: Array<string>;

  constructor(props: {}){
    super(props);
    this.state = {filter: {}, search: ""};

    this.featuredData = new Array<Project>;
    this.projectData = new Array<Project>;
    this.filterButtonTags = new Array<string>;

    this.read(); // Read project data and tags from database
  }

  FilterButtons = (): JSX.Element => {
    /* Renders the filter buttons at the top of the page using database tags.
       Class member due to use of this.handleChange. */
    const [checked] = useState(false);
    return(
      <Container fluid className="mb-5" id="filter-container">
        <p id="filter-label">Or filter by tag(s):</p>
        <ToggleButtonGroup type="checkbox" className="filter-btns">
          { /* Generate buttons from database tags. Key warnings can be
                safely ignored; this map won't change after being rendered. */
          this.filterButtonTags.map((tag:string) => (
            <ToggleButton
              id={"filter-" + tag}
              type="checkbox"
              checked={checked}
              value={tag}
              onChange={(e) => {this.handleFilterBtnChange(e)}}
            >
              {tag}
            </ToggleButton>))
          }
        </ToggleButtonGroup>
      </Container>
    );
  }

  handleFilterBtnChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    /* Updates this.state.filter based on the states of the filter buttons.
       resolveFilter() called by Section() on render, no need to call here. */
    // Use button name as a key, set boolean to checked state
    this.state.filter[e.currentTarget.value] = e.currentTarget.checked;
    this.setState(this.state); // Update render after updating filter
  }

  handleSearchBarChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    /* Updates this.state.search to the search bar contents on change. 
       search() called by Section() on render, no need to call here. */
    // Set to lowercase for case insensitive search.
    this.setState({search: e.target.value.toLowerCase()});
  }

  Section = (params: SectionParams): JSX.Element => {
    /* Renders a project section, or appropriate message if none rendered.
       Class member due to use of this.resolveFilter() and this.search(). */
    let displayed = false;
    return(
      <div className="projects-section mb-3">
        <h3 className="mb-3">{params.title}</h3>
        { // If data is not yet loaded from database:
        this.projectData.length == 0 &&
          <p>Loading projects from database...</p>
        }

        {/* Generate project cards from database info. Key warnings can be
            safely ignored; this map won't change after being rendered. */
          params.data.map((params:Project) => {
            // Update vis state based on output of filter and search functions
            params.vis = this.resolveFilter(params.tags) &&
                         this.search(params.tags, params.title);
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
      this.filterButtonTags.push(doc.id)
    });

    let data = await readProjectData(true); // all = true: Read all projects
    this.featuredData = data[0];
    this.projectData = data[1];

    this.setState(this.state); // Update render after reading from database
  }

  resolveFilter(tags: Array<string>): boolean{
    /* Helper function to determine whether a ProjectCard should be shown.
       Because of how params.vis is calculated, returning true is equivalent to
       calling search(), while returning false skips search(). */
    if (Object.keys(this.state.filter).length == 0) // Trivial case: Empty
      return true;

    let filtersDisabled = true;
    for (const key in this.state.filter){ // Check if any filters are enabled
      // Non-trivial case: Filter enabled, display card only if a tag matches
      if (this.state.filter[key] == true){
        filtersDisabled = false; // Flag that at least one filter is enabled
        // Search for the key inside the tags (partial match allowed)
        if (tags.some((element:string) => element.indexOf(key) != -1))
          return true; // True if search bar is empty
      }
    }
    /* Filter is enabled with no matches found (filterDisabled = false)
       or all filters are disabled (filterDisabled = true, go to search()). */
    return filtersDisabled;
  }

  search(tags: Array<string>, title: string): boolean{
    /* Helper function to determine whether a ProjectCard should be shown. 
       Only called if resolveFilter() returned true. */
    if (this.state.search.length == 0) // Trivial case: Empty
      return true; // Use output of resolveFilter() only

    // Search the title (case insensitive, partial match allowed)
    if (title.toLowerCase().includes(this.state.search))
      return true;

    // Search the tags (case insensitive, partial match allowed)
    if (tags.some((element:string) =>
        element.toLowerCase().indexOf(this.state.search) != -1))
      return true;

    return false; // No matches
  }

  render(){
    return(
      <main>
        <Form.Control
          className="mb-3"
          onChange={(e) => this.handleSearchBarChange(e as any)}
          placeholder="Search for projects by title, category, or tags..."
        />

        {/* Must be its own function to use "checked" hook */}
        <this.FilterButtons/>

        {/* Project sections. Data passed as reference, no memory waste. */}
        <div className="sections">
          <this.Section title="Featured Projects" data={this.featuredData}/>
          <this.Section title="Projects" data={this.projectData}/>
        </div>
      </main>
    );
  }
}