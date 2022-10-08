// РАДИО МОДУЛЬ

import {h, Component} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";

export default class Interface extends Component {
  constructor() {
    super();
    this.setState({
      expanded: {
        interface: false
      },
      themes: [
        "int.th.light",
        "int.th.dark"
      ]
    });
  }


  render({}, {themes}) {
    const {dls, setLang, setTheme} = this.context;

    return (

      <div className="sb">
        {/* ИНТЕРФЕЙС */}
        <input type="checkbox" id="interface" className="checker"
               checked={this.state.expanded.interface}
               onChange={this.linkState("expanded.interface")}/>
        <label for="interface"><p className="tsb"><Message key="int.title"/></p>
        </label>
        <div className="spl">

          {/* Язык интефейса */}
          <p className="bold"><Message key="int.lang"/></p>
          {dls.map(dl =>
            <Button
              className={"button grey large" + (dl === this.context.dl ? " active" : "")}
              onClick={setLang}
              value={dl}>
              {dl.toUpperCase()}
            </Button>
          )}

          {/* Тема оформления */}
          <p className="bold"><Message key="int.dv"/></p>
          {themes.map(dv =>
            <Button className={"button w30 blue large" + (themes.indexOf(dv) === parseInt(this.context.dv) ? " active" : "")}
                    onClick={setTheme}
                    value={themes.indexOf(dv)}>
              <Message key={dv}/>
            </Button>
          )}
        </div>
      </div>
    );
  }
}