// Страница "УПРАВЛЕНИЕ" (левая панель)

import {h, Component} from "preact";
import Message from "../components/Message.jsx";

export default class NetworkManager extends Component {
  render() {
    return (
      <div style="padding: 0 1em;">
        <h4 className="tPan"><Message key="control.title"/></h4>
        <h5 className={"indent justify"}><Message key="control.left.c1"/></h5>
      </div>
    );
  }
}
