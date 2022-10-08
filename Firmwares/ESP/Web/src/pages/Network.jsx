// Страница "НАСТРОЙКИ" (центр)

import {h, Component} from "preact";
import STAConfig from "../components/WiFiSTAConfig.jsx";
import RFConfig from "../components/RFConfig.jsx";
import APConfig from "../components/WiFiAPConfig.jsx";
import Message from "../components/Message.jsx";
import Button from "../components/Button.jsx";


export default class NetworkMain extends Component {


  render({}) {
    return (
      <div>
        <h3 className="tPan"><Message key="network.title"/></h3>
        <div>
          <STAConfig/>
          <RFConfig/>
          <APConfig/>
        </div>
        <Button className="button w75 blue large" onClick={() => this.context.sendWS("Rst")}><Message
          key="network.rst"/></Button>
        <br/>
        <br/>
      </div>
    );
  }
}
