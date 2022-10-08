// ОБНОВЛЕНИЯ ВСЕГО

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";
import Select from "./Select.jsx";

export default class UpdateALL extends Component {

  constructor() {
    super();
    this.setState({
      idItem: ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"]
    });
  }

  onIDChange1(event) {
    this.context.setParam("id1", event.target.value);
  }
  onIDChange2(event) {
    this.context.setParam("id2", event.target.value);
  }

  render({}, {idItem}) {
    const {updPrgMCU, updPrgFFS, updPrgESP, device} = this.context;
    const {id1, id2} = device;
    return (
        <div>
          {/* Обновление по ID (только для устройств SOI Tech) */}
          <p className="bold"><Message key="id.title"/></p>
          <Select id="idSelect1" className={"select w5 large"} onChange={this.onIDChange1.bind(this)} value={id1}>
            {idItem.map(id => <option value={id}>{id}</option>)}
          </Select>
          <Select id="idSelect2" className={"select w5 large"} onChange={this.onIDChange2.bind(this)} value={id2}>
            {idItem.map(id => <option value={id}>{id}</option>)}
          </Select>
          <Button className="button w50 blue large" onClick={() => this.context.sendWS("UpdESPStartID")}><Message key="id.upd"/></Button>
          <p><Message key="id.prgm"/>{updPrgMCU}% || <Message key="id.prgf"/>{updPrgFFS}% || <Message key="id.prgc"/>{updPrgESP}%</p>
        </div>
    );
  }
}
