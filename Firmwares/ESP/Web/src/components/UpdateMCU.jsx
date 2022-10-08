// ОБНОВЛЕНИЯ MCU

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";
import Input from "./Input.jsx";

export default class UpdateMCU extends Component {

  constructor() {
    super();
    this.setState({
      fileFlash: {"name": "Flash"},
      fileEEPROM: {"name": "EEPROM"}
    });
    this.onUploadFlash = this.onUploadFlash.bind(this);
    this.onUploadEEPROM = this.onUploadEEPROM.bind(this);
    this.onClear = this.onClear.bind(this);
  }

  onUploadFlash(event) {
    this.setState({fileFlash: event.target.files[0]});
    const {fileFlash} = this.state;
    const data = new FormData;
    data.append("data", fileFlash);
    fetch("/mcuFlash", {
      method: "POST",
      body: data
    })
      .catch(e => {
        console.log(e);
        alert( "Error upload MCU Flash" );
      });
  }

  onUploadEEPROM(event) {
    this.setState({fileEEPROM: event.target.files[0]});
    const {fileEEPROM} = this.state;
    const data = new FormData;
    data.append("data", fileEEPROM);
    fetch("/mcuEEPROM", {
      method: "POST",
      body: data
    })
      .catch(e => {
        console.log(e);
        alert( "Error upload MCU EEPROM" );
      });
  }

  onClear(event) {
    this.setState({fileFlash: {"name": "Flash"}});
    this.setState({fileEEPROM: {"name": "EEPROM"}});
    this.context.sendWS("UpdMCUClear");
  }

  render({}, {fileFlash, fileEEPROM}) {
    const {updPrgMCU} = this.context;
    return (
      <div>
        {/* ОБНОВЛЕНИЕ MCU */}
        <p className="bold"><Message key="mcu.title"/></p>
        <Button className="button w50 blue large" onClick={this.onClear}><Message key="mcu.clear"/></Button>

        <label className="fileform w75">
          <span>{fileFlash ? fileFlash["name"] : "Flash"}</span>
          <Input className="input large upload" type="file" name="upload" id="uploadFlash" onChange={this.onUploadFlash}/>
        </label>

        <label className="fileform w75">
          <span>{fileEEPROM ? fileEEPROM["name"] : "EEPROM"}</span>
          <Input className="input large upload" type="file" name="upload" id="uploadEEPROM" onChange={this.onUploadEEPROM}/>
        </label>

        <Button className="button w50 blue large" onClick={() => this.context.sendWS("UpdMCUStart")}><Message key="mcu.upd"/></Button>
        <p><Message key="mcu.prg"/>{updPrgMCU}%</p>
      </div>
    );
  }
}
