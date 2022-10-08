import {h} from "preact";

const Input = ({children, ...rest}) => (<input {...rest}>{children}</input>);

export default Input;
