// Страница "СЕТЬ" (левая панель)

import {h, Component} from "preact";
import Message from "../components/Message.jsx";

export default class NetworkManager extends Component {
  render() {
    return (
      <div style="padding: 0 1em;">
        <h4 className={"tPan"}><Message key="manager.left.title"/></h4>
        <h5 className={"indent justify"}><Message key="manager.left.c1"/></h5>
        <h5 className={"indent justify"}><Message key="manager.left.c2"/></h5>
        <h5 className={"indent justify"}><Message key="manager.left.c3"/></h5>
      </div>
    );
  }
}
