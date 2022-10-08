// СЕРВИС

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";
import SendAuthorization from "./SendAuthorization.jsx"
import UpdateByID from "./UpdateByID.jsx";

export default class Service extends Component {

  constructor() {
    super();
    this.setState({
      service: {},
      expanded: {service: false}
    });
  }


  render({}) {
    const {infoMessage} = this.context;

    return (
      <div className="sb">
        {/* СЕРВИС */}
        <input type="checkbox" id="service" className="checker" checked={this.state.expanded.service} onChange={this.linkState("expanded.service")}/>
        <label for="service"><p className="tsb"><Message key="srv.title"/></p></label>
        <div className="spl">
          {/* РАССЫЛКА ПАРАМЕТРОВ АВТОРИЗАЦИИ */}
          <SendAuthorization className="button w50 blue large" onClick={() => this.context.sendWS("Synch")}/>
          <hr/>
          {/* ОБНОВЛЕНИЕ УСТРОЙСТВА ПО ID */}
          <UpdateByID/>
          <p className="bold">{infoMessage}</p>
          <hr/>
          {/* СБРОС НАСТРОЕК */}
          <p className="bold"><Message key="srv.def"/></p>
          <Button className="button w50 red large" onClick={() => this.context.sendWS("DefSettings")}><Message
            key="srv.perform"/></Button>
        </div>
      </div>
    );
  }
}
