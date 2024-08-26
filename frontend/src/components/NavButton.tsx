import {useNavigate} from 'react-router-dom';

import Button from 'react-bootstrap/Button';

interface ButtonParams{
  /* Props interface for NavButton(). */
  // Link for button to navigate to.
  href: string;
  // Text to display on button.
  children: string;
  // react-bootstrap variant to use. Defaults to primary.
  variant?: string;
}

export default function NavButton(params: ButtonParams){
  const navigate = useNavigate();

  return(
    <Button
      variant={params.variant ?? "primary"}
      onClick={() => navigate(params.href)}
    >
      {params.children}
    </Button>
  );
}