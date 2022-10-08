// WIFI МОДУЛЬ STA

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";
import Select from "./Select.jsx";
import Input from "./Input.jsx";

export default class RFConfig extends Component {
  constructor() {
    super();
    this.setState({
      expanded: {
        sta: false
      },
      manualStaSSID: true
    });
    this.onSTAChange = this.onSTAChange.bind(this);
    this.onSTAIPModeChange = this.onSTAIPModeChange.bind(this);
    this.scanNetAPs = this.scanNetAPs.bind(this);
  }

  componentWillMount() {
    this.scanNetAPs();
  }

  onSTAChange(event) {
    if (event.target.value === "") {
      this.setState({manualStaSSID: true});
    } else {
      this.setState({manualStaSSID: false});
      this.context.setParam("staSSID", event.target.value);
    }
  }

  onSTAIPModeChange(event) {
    this.context.setParam("staIPMode", event.target.checked === true ? 1 : 0);
  }

  scanNetAPs() {
    this.context.sendWS("NetList");
    setTimeout(() => {
      if (this.context.device.staSSID.length) {
        this.setState({manualStaSSID: false});
      } else {
        this.setState({manualStaSSID: true});
      }
    }, 3000);
  }

  render({}) {
    const {device, linkState, NetList} = this.context;
    const {staSSID, staPSK, staIPMode, staIP, staMS, staGW} = device;

    return (
      <div className="sb">
        {/* РЕЖИМ "КЛИЕНТ" */}
        <input type="checkbox" id="sta" className="checker"
               checked={this.state.expanded.sta}
               onChange={this.linkState("expanded.sta")}/>
        <label for="sta"><p className="tsb"><Message key="sta.title"/></p></label>
        <div className="spl">
          <p className="bold"><Message key="sta.ssid"/></p>

          {/* SSID */}
          <Select id="staSSIDSelect"
                  className={"select w77 large" + (this.state.manualStaSSID ? " hidden" : "")}
                  onChange={this.onSTAChange} value={staSSID}>
            {NetList.map(ap =>
              <option value={ap.ssid}>{`${ap.ssid} (${ap.rssi}dBm, Ch${ap.ch})`}</option>
            )}
            <option disabled>================</option>
            <option value=""><Message key="sta.hidden"/></option>
          </Select>
          <Input id="staSSID" type="text" maxlength="32"
                 className={"input w75 large" + (this.state.manualStaSSID ? "" : " hidden")}
                 onInput={linkState("device.staSSID")} value={staSSID}/>

          <Button className="button w50 blue large" onClick={() => this.scanNetAPs()}>
            <Message key="sta.scan"/>
          </Button>

          {/* Пароль */}
          <p className="bold"><Message key="sta.psk"/></p>
          <Input type="text" className="input w75 large" maxlength="64"
                 onInput={linkState("device.staPSK")}
                 value={staPSK}/>
          <br/><br/>

          {/* Дополнительные параметры сети */}
          <p className="bold"><Message key="sta.advpar"/></p>
          <input type="checkbox" id="staAdv" className="checker"
                 checked={staIPMode}
                 onChange={this.onSTAIPModeChange}/>
          <label for="staAdv" className="toggle"/>
          <div className="spl">
            <div className="adv-param">

              {/* IP адрес */}
              <p><Message key="sta.address"/></p>
              <Input className="input w50 large" value={staIP} type="text" maxlength="15"
                     onInput={linkState("device.staIP")}/>

              {/* Маска подсети */}
              <p><Message key="sta.netmask"/></p>
              <Input className="input w50 large" value={staMS} type="text" maxlength="15"
                     onInput={linkState("device.staMS")}/>

              {/* IP адрес шлюза */}
              <p><Message key="sta.gateway"/></p>
              <Input className="input w50 large" value={staGW} type="text" maxlength="15"
                     onInput={linkState("device.staGW")}/>

              <p className="italic"><Message key="sta.ipinfo1"/></p>
              <p className="italic"><Message key="sta.ipinfo2"/></p>
            </div>
          </div>
          <br/>
          <Button className="button w50 blue large" onClick={() => this.context.sendWS("SetSTA")}>
            <Message key="sta.save"/>
          </Button>
        </div>
      </div>
    );
  }
}