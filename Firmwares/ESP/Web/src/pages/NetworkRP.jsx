// Страница "НАСТРОЙКИ" (правая панель)

import {h, Component} from "preact";
import Message from "../components/Message.jsx";

export default class Parameters extends Component {
  render() {
    return (
      <div style="padding: 0 1em;">
        <h4 className="tPan ltr"><Message key="network.right.title"/></h4>
        <h5 className={"indent justify"}><Message key="network.right.c1"/></h5>
        <h5 className={"indent justify"}><Message key="network.right.c2"/></h5>
        <h5 className={"indent justify"}><Message key="network.right.c3"/></h5>
        <h5 className={"indent justify"}><Message key="network.right.c4"/></h5>
        <h5 className={"indent justify"}><Message key="network.right.c5"/></h5>
      </div>
    );
  }
}