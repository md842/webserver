import React from 'react';
import {Navbar, Container, Nav, Button} from 'react-bootstrap';

import './Header.css';

import moon from '../assets/moon.svg';
import sun from '../assets/sun.svg';

interface DarkModeState{
  icon: string;
}

export default class Header extends React.Component<{}, DarkModeState>{
  constructor(props: {}) {
    super(props);
    this.state = {icon: moon};
  }

  switchColorMode(){
    if (this.state.icon === sun){ // Switch to light mode
      this.setState({icon: moon});
      document.documentElement.setAttribute('data-bs-theme', "light");
    }
    else{ // Switch to dark mode
      this.setState({icon: sun});
      document.documentElement.setAttribute('data-bs-theme', "dark");
    }
  }

  render(){
    return(
      <>
        <Navbar sticky="top" bg="dark" data-bs-theme="dark">
          <Container className="nav-container">
            <Nav variant="underline" activeKey={window.location.pathname}> {/* Highlights active page in nav bar */}
              <Navbar.Brand href="/">Max Deng</Navbar.Brand>
              <Nav.Link href="/">Home</Nav.Link>
              <Nav.Link href="/projects">Projects</Nav.Link>
              <Nav.Link href="/resume">Resume</Nav.Link>
            </Nav>
            <Button
              variant="link"
              onClick={() => this.switchColorMode()}
            >
              <img src={this.state.icon} className="dark-mode-svg"/>
            </Button>
          </Container>
        </Navbar>
      </>
    );
	}
}