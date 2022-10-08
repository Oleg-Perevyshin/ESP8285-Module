// Страница "НАСТРОЙКИ" (левая панель)

import {h, Component} from "preact";
import Message from "../components/Message.jsx";

export default class Parameters extends Component {
  render() {
    return (
      <div style="padding: 0 1em;">
        <h4 className="tPan ltr"><Message key="network.left.title"/></h4>
        <h5 className={"indent justify"}><Message key="network.left.c1"/></h5>
        <h5 className={"indent justify"}><Message key="network.left.c2"/></h5>
      </div>
    );
  }
}
