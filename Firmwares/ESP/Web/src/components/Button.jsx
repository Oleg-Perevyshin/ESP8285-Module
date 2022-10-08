import {h} from "preact";

const Button = ({children, ...rest}) => (<button {...rest}>{children}</button>);

export default Button;
