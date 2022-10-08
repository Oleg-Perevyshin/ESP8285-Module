// ИДЕНТИФИКАЦИЯ

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";
import Input from "./Input.jsx";
import {stringify} from "querystring";

export default class IDConfig extends Component {
  constructor() {
    super();
    this.setState({
      colors: ["d15", "f39", "f72", "fc1", "1b0", "5ec", "2bf", "27f", "92f", "999"],
      expanded: {
        id: false
      }
    });
  }


  render({}, {colors}) {
    const {device, linkState, setParam} = this.context;
    const {dn, du, dp} = device;

    return (
      <div className="sb">
        <input type="checkbox" id="id" className="checker"
               checked={this.state.expanded.id}
               onChange={this.linkState("expanded.id")}/>
        <label for="id"><p className="tsb"><Message key="ident.title"/></p></label>
        <div className="spl">
          {/* Имя устройства */}
          <p className="bold"><Message key="ident.name"/></p>
          <Input className="input w75 large" value={dn} type="text" maxlength="16"
                 onInput={linkState("device.dn")}/>

          {/* Выбор цветовой зоны */}
          <p className="bold"><Message key="ident.color"/></p>
          {colors.map(color =>
            <a className={"cDev" + (device.dc === color ? " active" : "")}
               style={`background: #${color}`}
               onClick={() => setParam("dc", color)}/>
          )}
          {/* Имя пользователя и пароль к Web интерфейсу */}
          <p className="bold"><Message key="ident.user"/></p>
          <Input className="input w75 large" value={du} type="text" maxlength="16"
                 onInput={linkState("device.du")}/>
          <p className="bold"><Message key="ident.psk"/></p>
          <Input className="input w75 large" value={dp} type="text" maxlength="16"
                 onInput={linkState("device.dp")}/>
          <Button className="button w50 blue large" onClick={() => this.context.sendWS("SetID")}><Message
            key="ident.save"/></Button>
          <p className="italic"><Message key="ident.max16"/></p>
        </div>
      </div>
    );
  }
}