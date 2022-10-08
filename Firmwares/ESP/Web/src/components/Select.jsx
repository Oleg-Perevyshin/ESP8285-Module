import {h} from "preact";

const Select = ({children, ...rest}) => (<select {...rest}>{children}</select>);

export default Select;
