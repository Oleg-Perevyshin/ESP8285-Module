// WIFI МОДУЛЬ AP

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";
import Input from "./Input.jsx";


export default class RFConfig extends Component {
  constructor() {
    super();
    this.setState({
      expanded: {
        ap: false,
        apAdv: false
      }
    });
  }

  componentWillMount() {

  }

  // onInput={this.isIP.bind(this)}
  // isIP(event) {
  //   const ipformat = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
  //   if(event.value.match(ipformat)) {
  //     return true;
  //   } else {
  //     const {scripts} = {...this.state};
  //     scripts[index].AP = event.target.value.replace(/[^0-9]/g, "").substring(0, 4);
  //     this.setState({scripts});
  //   }
  // }


  render({}) {
    const {device, linkState} = this.context;
    const {apSSID, apPSK, apIP, apMS, apGW} = device;

    return (
      <div className="sb">
        {/* РЕЖИМ "ТОЧКА ДОСТУПА" */}
        <input type="checkbox" id="ap" className="checker"
               checked={this.state.expanded.ap}
               onChange={this.linkState("expanded.ap")}/>
        <label for="ap"><p className="tsb"><Message key="ap.title"/></p>
        </label>
        <div className="spl">

          {/* Имя модуля в режиме "Точка Доступа" */}
          <p className="bold"><Message key="ap.ssid"/></p>
          <Input type="text" className="input w75 large" value={apSSID} maxlength="16"
                 onInput={linkState("device.apSSID")}/>

          {/* Пароль */}
          <p className="bold"><Message key="ap.psk"/></p>
          <Input type="text" className="input w75 large" value={apPSK} maxlength="16"
                 onInput={linkState("device.apPSK")}/>

          {/* Дополнительные параметры сети */}
          <p className="bold"><Message key="ap.advpar"/></p>
          <input type="checkbox" id="apAdv" className="checker"/>
          <label for="apAdv" className="toggle"/>
          <div className="spl">
            <div className="adv-param">

              {/* IP адрес */}
              <p><Message key="ap.address"/></p>
              <Input className="input w50 large" value={apIP} type="text" maxlength="15"
                     onInput={linkState("device.apIP")}/>

              {/* Маска подсети */}
              <p><Message key="ap.netmask"/></p>
              <Input className="input w50 large" value={apMS} type="text" maxlength="15"
                     onInput={linkState("device.apMS")}/>

              {/* IP адрес шлюза */}
              <p><Message key="ap.gateway"/></p>
              <Input className="input w50 large" value={apGW} type="text" maxlength="15"
                     onInput={linkState("device.apGW")}/>

              <p className="italic"><Message key="ap.ipinfo1"/></p>
              <p className="italic"><Message key="ap.ipinfo2"/></p>
            </div>
          </div>
          <br/>
          <Button className="button w50 blue large" onClick={() => this.context.sendWS("SetAP")}>
            <Message key="ap.save"/>
          </Button>
          <p className="italic"><Message key="ap.max16"/></p>
        </div>
      </div>
    );
  }
}