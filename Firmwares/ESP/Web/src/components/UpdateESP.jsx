// ОБНОВЛЕНИЯ ESP

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";

export default class UpdateESP extends Component {

  constructor() {
    super();
    this.UpdCheck = this.UpdCheck.bind(this);
  }

  componentWillMount() {
    this.UpdCheck();
  }

  UpdCheck() {
    this.context.sendWS("UpdCheck");
    setTimeout(() => {
      if (this.context.UpdCheck) {
        //alert('A new update is available.\nVisit the "Service" section')
      }
    }, 1500);
  }

  render({}) {
    const {UpdCheck, updPrgFFS, updPrgESP} = this.context;

    return (
      <div>
        {/* ОБНОВЛЕНИЕ */}
        <p className="bold"><Message key="esp.title"/></p>
        {UpdCheck ?
          <div>
            <Button className="button w50 blue large" onClick={() => this.context.sendWS("UpdESPStart")} disabled={!UpdCheck}><Message key="esp.upd"/></Button>
            <p><Message key="esp.prgf"/>{updPrgFFS}% || <Message key="esp.prgc"/>{updPrgESP}%</p>
          </div>
          :
          <div>
            <p className="bold"><Message key="esp.nupd"/></p>
          </div>
        }
      </div>
    );
  }
}
