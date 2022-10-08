// РАДИО МОДУЛЬ

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";
import Input from "./Input.jsx";

export default class IOs extends Component {
  constructor() {
    super();
    this.setState({
      expanded: {
        io: false
      }
    });
  }


  render({}) {
    const {device, linkState} = this.context;
    const {in1, in2} = device;

    return (
      <div className="sb">
        {/* ИМЕНОВАНИЕ ВЫВОДОВ */}
        <input type="checkbox" id="io" className="checker"
               checked={this.state.expanded.io}
               onChange={this.linkState("expanded.io")}/>
        <label for="io"><p className="tsb"><Message key="io.title"/></p></label>
        <div className="spl">
          <p className="bold">
            <span><Message key="io.in1"/></span>&nbsp;
            <Input type="text" className="input w30 large" maxlength="16" value={in1}
                   onInput={linkState("device.in1")}/>&nbsp;&nbsp;
            <span><Message key="io.in2"/></span>&nbsp;
            <Input type="text" className="input w30 large" maxlength="16" value={in2}
                   onInput={linkState("device.in2")}/>
          </p>
          <Button className="button w50 blue large" onClick={() => this.context.sendWS("IOs")}><Message
            key="io.save"/></Button>
          <p className="italic"><Message key="io.max16"/></p>
        </div>
      </div>
    );
  }
}