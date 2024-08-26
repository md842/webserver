import './Header.css';

import {useEffect, useState} from 'react';
import {Link, useLocation} from 'react-router-dom';

import {Navbar, Container, Nav, Button} from 'react-bootstrap';

import moon from '../assets/moon.svg';
import sun from '../assets/sun.svg';

export default function Header(){
  const [icon, setIcon] = useState(moon);
  const [mode, setMode] = useState("light");

  useEffect(() => {
    const storedMode = window.localStorage.getItem('mode');
    if (storedMode){
      setMode(storedMode);
      document.documentElement.setAttribute('data-bs-theme', storedMode);
      if (storedMode == "light") // Switch to light mode
        setIcon(moon);
      else // Switch to dark mode
        setIcon(sun);
    }
  }, []);

  useEffect(() => {
    window.localStorage.setItem('mode', mode);
  }, [mode]);

  const switchColorMode = () => {
    if (mode == "dark"){ // Switch to light mode
      setIcon(moon);
      setMode("light");
      document.documentElement.setAttribute('data-bs-theme', "light");
    }
    else{ // Switch to dark mode
      setIcon(sun);
      setMode("dark");
      document.documentElement.setAttribute('data-bs-theme', "dark");
    }
  }

  return(
    <header>
      <Navbar sticky="top" bg="dark" data-bs-theme="dark">
        <Container className="nav-container">
          <Nav
            activeKey={'/' + useLocation().pathname.split('/')[1]}
            variant="underline"
          > {/* Highlights active page in nav bar */}
            <Navbar.Brand as={Link} to="/">Max Deng</Navbar.Brand>
            <Nav.Link as={Link} eventKey="/" to="/">Home</Nav.Link>
            <Nav.Link as={Link} eventKey="/projects" to="/projects">Projects</Nav.Link>
          </Nav>
          <Button
            variant="link"
            onClick={() => switchColorMode()}
          >
            <img src={icon} className="dark-mode-svg"/>
          </Button>
        </Container>
      </Navbar>
    </header>
  );
}