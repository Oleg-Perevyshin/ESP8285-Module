import {translate} from "../util.jsx";

const Message = ({key}, {dictionary}) => translate(dictionary, key);

export default Message;
