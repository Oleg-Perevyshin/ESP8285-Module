// Страница "УПРАВЛЕНИЕ" (центр)

import {h, Component} from "preact";
import Loading from "../components/Loading.jsx";
import Message from "../components/Message.jsx";
import Button from "../components/Button.jsx";
import UpdateESP from "../components/UpdateESP.jsx";
import UpdateMCU from "../components/UpdateMCU.jsx";
import {fetchJSON, fetchText} from "../util.jsx";
import {stringify} from "querystring"
import {Slider, MultiSlider} from 'preact-range-slider';

export default class ControlPage extends Component {
  constructor() {
    super();
    this.state = {
      settings: {}
    };
    this.setParam = this.setParam.bind(this);
  }

  componentWillMount() {
    this.setState({
      expanded: {
        upd: false
      },
      mainModes: [
        "c.m.mode.0",
        "c.m.mode.1",
        "c.m.mode.2"
      ]
    });
    this.context.sendWS("Settings");
    this.onChangeMode = this.onChangeMode.bind(this);
  }

  // Передача параметра (id=value)
  setParam(event) {
    this.setState({Settings: {...this.state.Settings, [event.target.id]: event.target.value}});
    this.context.sendWS(event);
  }

  onChangeMode(event) {
    this.setParam(event);
  }


  render({}, {mainModes}) {
    const {linkState, infoMessage} = this.context;
    return (
      <div>
        <h3 className="tPan"><Message key="control.title"/></h3>
        <div className="sb">
          {/* ОСНОВНЫЕ ПАРАМЕТРЫ */}
          <input type="checkbox" id="upd" className="checker"
                 checked={this.state.expanded.upd}
                 onChange={this.linkState("expanded.upd")}/>
          <label for="upd"><p className="tsb"><Message key="control.upd"/></p></label>
          <div className="spl">

            <UpdateESP/>
            <hr/>
            <UpdateMCU/>
            <p className="bold">{infoMessage}</p>

          </div>
        </div>
      </div>
    );
  }
}

// Example
// const {Settings, linkState, infoMessage} = this.context;
// const {rs, rm} = Settings;
// {/* Режим работы */}
//                 <p className="bold"><Message key="c.m.mode.t1"/></p>
//                 {mainModes.map(item =>
//                   <Button id="gmd" className={"button w30 blue large" +
//                   (mainModes.indexOf(item) === parseInt(this.state.settings.gmd) ? " active" : "")
//                   }
//                           onClick={this.onChangeMode}
//                           value={mainModes.indexOf(item)}>
//                     <Message key={item}/>
//                   </Button>
//                 )}
// <MultiSlider style={"margin: 1em; width: 95%;"}
//             min={0} max={5} step={0.5} value={rm}
//             pushable={1}
//             allowCross={false}
//             defaultValue={[0, 1]}
//             marks={[0, 1, 2, 3, 4, 5]}
//             onChange={linkState("Settings.rm")}
//             onAfterChange={(value) => this.context.sendWS("rm", value)}
// />

// <Slider style={"margin: 1em; width: 95%;"}
//        min={0} max={16} step={1} value={rs}
//        defaultValue={12}
//        marks={[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]}
//        onChange={linkState("Settings.rs")}
//        onAfterChange={(value) => this.context.sendWS("rs", value)}
// />
