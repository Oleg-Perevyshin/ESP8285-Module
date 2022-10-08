// РАДИО МОДУЛЬ

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";
import Select from "./Select.jsx";
import Input from "./Input.jsx";

export default class RFConfig extends Component {
  constructor() {
    super();
    this.setState({
      rchs: ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25"],
      expanded: {
        rf: false
      }
    });
    this.onRfChChange = this.onRfChChange.bind(this);
  }

  onRfChChange(event) {
    this.context.setParam("rch", event.target.value);
  }


  render({}, {rchs}) {
    const {device, linkState} = this.context;
    const {da, rch} = device;

    return (
      <div className="sb">
        <input type="checkbox" id="rf" className="checker"
               checked={this.state.expanded.rf}
               onChange={this.linkState("expanded.rf")}/>
        <label for="rf"><p className="tsb"><Message key="rf.title"/></p></label>
        <div className="spl">
          {/* ID устройства в радио сети */}
          <p className="bold"><Message key="rf.da"/></p>
          <Input className="input w75 large" value={da} type="text" maxlength="6"
                 onInput={linkState("device.da")}/>
          <p className="bold"><Message key="rf.ch"/></p>
          <Select id="rfChSelect"
                  className={"select w77 large"}
                  onChange={this.onRfChChange} value={rch}>
            {rchs.map(rch =>
              <option value={rch}>{rch}</option>
            )}
          </Select>
          <br/><br/>
          <Button className="button w50 blue large" onClick={() => this.context.sendWS("SetRF")}><Message key="rf.save"/></Button>
        </div>
      </div>
    );
  }
}