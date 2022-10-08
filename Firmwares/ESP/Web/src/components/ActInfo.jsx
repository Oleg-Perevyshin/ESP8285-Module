// ИНФО

import {h, Component} from "preact";
import Message from "./Message.jsx";

export default class RFConfig extends Component {
  constructor() {
    super();
    this.setState({
      expanded: {
        info: false
      }
    });
  }

  componentWillMount() {
    this.context.sendWS("ActualInfo");
  }

  render({}) {
    const {ActualInfo} = this.context;
    const {
      actID, actDM, actDN, actV, actLV, actDU, actDP,
      actStaSSID, actStaPSK, actWRSSI, actWCH, actStaIP, actStaMS, actStaGW, actStaMAC,
      actApSSID, actApPSK, actApIP, actApMS, actApGW, actApMAC,
      actDA, actRCH
    } = ActualInfo;

    return (
      <div className="sb">
        <input type="checkbox" id="info" className="checker"
               checked={this.state.expanded.info}
               onChange={this.linkState("expanded.info")}/>
        <label for="info"><p className="tsb"><Message key="info.title"/></p></label>
        <div className="spl">
          <div className="block-in">
            <p className="bold"><Message key="info.general"/></p>
            <p><Message key="info.device"/>{actDM} | {actDN}</p>
            <p><Message key="info.di"/>{actID}</p>
            <p><Message key="info.curver"/>{actV} | <Message key="info.latver"/>{actLV}</p>
            <p><Message key="info.user"/>{actDU}</p>
            <p><Message key="info.psk"/>{actDP}</p>
          </div>
          <div className="block-in">
            <p className="bold"><Message key="info.sta"/></p>
            <p><Message key="info.ssid"/>{actStaSSID}</p>
            <p><Message key="info.psk"/>{actStaPSK}</p>
            <p><Message key="info.rssi"/>{actWRSSI} | <Message key="info.ch"/>{actWCH}</p>
            <p><Message key="info.ip"/>{actStaIP}</p>
            <p><Message key="info.mask"/>{actStaMS}</p>
            <p><Message key="info.gateway"/>{actStaGW}</p>
            <p><Message key="info.mac"/>{actStaMAC}</p>
            <p className="bold"><Message key="info.ap"/></p>
            <p><Message key="info.ssid"/>{actApSSID}</p>
            <p><Message key="info.psk"/>{actApPSK}</p>
            <p><Message key="info.ip"/>{actApIP}</p>
            <p><Message key="info.mask"/>{actApMS}</p>
            <p><Message key="info.gateway"/>{actApGW}</p>
            <p><Message key="info.mac"/>{actApMAC}</p>
          </div>
          <div className="block-in">
            <p className="bold"><Message key="info.radio"/></p>
            <p><Message key="info.da"/>{actDA}</p>
            <p><Message key="info.ch"/>{actRCH}</p>
          </div>
        </div>
      </div>
    );
  }
}