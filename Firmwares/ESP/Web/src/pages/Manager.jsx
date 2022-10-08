// Страница "СЕТЬ" (центр)

import {h, Component} from "preact";
import Message from "../components/Message.jsx";
import Button from "../components/Button.jsx";
import Loading from "../components/Loading.jsx";

const getUniqueValues = (value, data) => {
  const uniqueColors = [];
  for (let item in data) {
    if ((value in data[item]) && (uniqueColors.indexOf(data[item][value]) === -1)) uniqueColors.push(data[item][value]);
  }
  return uniqueColors;
};

export default class ManagerMain extends Component {

  constructor() {
    super();
    this.setState({
      loading: false
    });
    this.scan = this.scan.bind(this);
    this.getIcon = this.getIcon.bind(this);
    this.getDev = this.getDev.bind(this);
  }

  componentWillMount() {
    this.context.sendWS("DevsList");
  }

  scan() {
    this.setState({loading: true});
    this.context.sendWS("DevScan");
    setTimeout(() => {
      this.setState({loading: false});
    }, 5000);
  }

  getIcon(ip) {
    fetch(`//${ip}/icon`)
      .then(response => {
        return response.text()
          .then(content => {
            document.getElementById(`icon_${ip}`).innerHTML = content;
          });
      });
  }

  getDev(ip) {
    fetch(`//${ip}/dev`)
      .then(response => {
        return response.text()
          .then(content => {
            document.getElementById(`cont_${ip}`).innerHTML = content;
          });
      });
  }

  render() {

    const {DevsList, colorSorting, setColorSorting, typeSorting, setTypeSorting, clearSorting} = this.context;

    return (
      <div>
        <h3 className="tPan"><Message key="manager.title"/></h3>
        {this.state.loading ? <Loading/> : (
          <div>
            <div className="block-in">
              <br/>
              <Button className="button w75 red large" onClick={this.scan}><Message key="manager.scan"/></Button>
              <h2 className="bold"><Message key="manager.select"/></h2>
              <Button className="button w75 blue large" onClick={clearSorting}><Message key="manager.alldevs"/></Button>
              <p><Message key="manager.type"/></p>
              {
                getUniqueValues("dt", DevsList).map(type => (
                  <Button className="button w30 gray large" onClick={() => setTypeSorting(type)}>{type}</Button>)
                )
              }
              <p><Message key="manager.color"/></p>
              {
                getUniqueValues("dc", DevsList).map(color => (
                  <a className="cDev" style={`background: #${color}`} onClick={() => setColorSorting(color)}/>)
                )
              }
            </div>
            <div id="data">
              {/* Формирователь блоков устройств с учетом фильтров селектора */}
              {
                Object.keys(DevsList)
                  .filter(key => {
                    let device = DevsList[key];
                    if (colorSorting || typeSorting) {
                      return colorSorting && device.dc === colorSorting || typeSorting && device.dt === typeSorting;
                    }
                    return true;
                  })

                  .map(device =>
                    <div className="block" id={DevsList[device].ip}
                         style={`border-top: .5em solid #${DevsList[device].dc}`}>

                      {/* Заголовок устройства */}
                      <div className="dev-wrp">
                        <div className="dev-icon" id={`icon_${DevsList[device].ip}`}>
                          {this.getIcon(`${DevsList[device].ip}`)}
                        </div>
                        <div className="dev-title">
                          <a target="_blank" className="bold"
                             href={`//${DevsList[device].ip}/#control`}>{DevsList[device].dm} | {DevsList[device].dn}</a>
                        </div>
                      </div>

                      {/* Кнопка сворачивания расширенных данных */}
                      <input type="checkbox" id={DevsList[device].da} className="checker"/>
                      <label for={DevsList[device].da} className="toggle"/>
                      <div id={`cont_${DevsList[device].ip}`} className="devhtm spl">
                        {this.getDev(`${DevsList[device].ip}`)}
                      </div>

                      {/* Низ блока устройства */}
                      <div className="dev-bottom">IP: {DevsList[device].ip} || SOI-RF: {DevsList[device].da}</div>

                    </div>
                  )
              }
            </div>
          </div>
        )}

      </div>
    );
  }
}
