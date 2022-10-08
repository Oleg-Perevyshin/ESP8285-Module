import {h, Component} from "preact";
import ManagerMain from "../pages/Manager.jsx";
import ManagerLeft from "../pages/ManagerLP.jsx";
import ManagerRight from "../pages/ManagerRP.jsx";
import NetworkMain from "../pages/Network.jsx";
import NetworkLeft from "../pages/NetworkLP.jsx";
import NetworkRight from "../pages/NetworkRP.jsx";
import ControlPage from "../pages/Control.jsx";
import ControlLP from "../pages/ControlLP.jsx";
import ControlRP from "../pages/ControlRP.jsx";
import ScriptsPage from "../pages/Scripts.jsx";
import ScriptsLP from "../pages/ScriptsLP.jsx";
import ScriptsRP from "../pages/ScriptsRP.jsx";
import SettingsMain from "../pages/Settings.jsx";
import SettingsLeft from "../pages/SettingsLP.jsx";
import SettingsRight from "../pages/SettingsRP.jsx";
import {fetchJSON, fetchText} from "../util.jsx";
import {stringify} from "querystring";
import Message from "./Message.jsx";
import Body from "./Body.jsx";
import autobind from "autobind-decorator";

@autobind
class App extends Component {
  constructor() {
    super();
    this.setState({
      ActualInfo: {},         // Актуальная информация об устройстве
      Settings: {},           // Настройки устройства
      dl: null,               // Язык интерфейса
      dls: [],                // Список языков (массив языковых файлов в папке /dl без расширения [ru,en,fr...])
      dv: null,               // Тема оформления интерфейса (число)
      NetList: [],            // Список точек доступа (масив объектов, в каждом объекте инфо об SSID, RSSI, Channel)
      dictionary: {},         // Словарь текущего языка (JSON файл соответствующего языка)
      device: {},             // Данные о текущем устройстве (объект с IP Адрес, Адрес радио сети, Тип, Модель, Имя, Цветовая зона устройства)
      DevsList: [],           // Список найденных устройствт компании SOI Tech (массив объектов, в каждом объекте данные как в device (выше))
      colorSorting: null,     // Цвет для сортировщика по Цвету
      typeSorting: null,      // Тип для сортировщика по Типу
      loading: true,          // Анимация загрузки
      ws: new WebSocket("ws://" + location.hostname + "/ws", ["soi-tech"]), // Web Socket
      attempts: 10,           // Попытки восстановления подключения Web Socket
      UpdCheck: false,        // Наличие обновления на сервере
      update: {},             //
      updPrgMCU: "0",         // Прогресс обновления при прошивке MCU (данные идут через Web Socket)
      updPrgFFS: "0",         // Прогресс обновления при прошивке FFS (данные идут через Web Socket)
      updPrgESP: "0",         // Прогресс обновления при прошивке Core ESP (данные идут через Web Socket)
      infoMessage: ""         // Информационные сообщения (данные идут через Web Socket)
    });
  }

  getChildContext() {
    return {
      root: this,
      setParam: ::this.setParam,
      linkState: ::this.linkState,
      sendWS: ::this.sendWS,
      infoMessage: this.state.infoMessage,
      setLang: ::this.setLang,
      setTheme: ::this.setTheme,
      ActualInfo: this.state.ActualInfo,
      Settings: this.state.Settings,
      dl: this.state.dl,
      dls: this.state.dls,
      dv: this.state.dv,
      NetList: this.state.NetList,
      colorSorting: this.state.colorSorting,
      typeSorting: this.state.typeSorting,
      setColorSorting: ::this.setColorSorting,
      setTypeSorting: ::this.setTypeSorting,
      clearSorting: ::this.clearSorting,
      dictionary: this.state.dictionary,
      device: this.state.device,
      DevsList: this.state.DevsList,
      UpdCheck: this.state.UpdCheck,
      update: this.state.update,
      updPrgMCU: this.state.updPrgMCU,
      updPrgFFS: this.state.updPrgFFS,
      updPrgESP: this.state.updPrgESP
    };
  }

  // Изменение состояния устройства
  setParam(param, value) {
    const {device} = this.state;
    device[param] = value;
    this.setState({device});
  }

  // Сортировка по цвету
  setColorSorting(value) {
    this.setState({colorSorting: value, typeSorting: null});
  }

  // Сортировка по типу
  setTypeSorting(value) {
    this.setState({colorSorting: null, typeSorting: value});
  }

  // Сброс параметров сортировки
  clearSorting() {
    this.setState({colorSorting: null, typeSorting: null});
  }

  componentDidMount() {
    this.setupWebsocket();
  }

  componentWillUnmount() {
    this.shouldReconnect = false;
    clearTimeout(this.timeoutID);
    let websocket = this.state.ws;
    websocket.close();
  }

  setupWebsocket() {
    let websocket = this.state.ws;

    websocket.onopen = () => {
    };

    // ОБРАБОТКА ВХОДЯЩИХ ПАКЕТОВ
    websocket.onmessage = (evt) => {
      const data = JSON.parse(evt.data);
      if (data.ActualInfo) {                                        // АКТУАЛЬНЫЕ ПАРАМЕТРЫ УСТРОЙСТВА
        this.setState({ActualInfo: JSON.parse(data.ActualInfo)});
      } else if (data.Settings) {                                   // НАСТРОЙКИ УСТРОЙСТВА
        this.setState({Settings: JSON.parse(data.Settings)});
      } else if (data.NetList) {                                    // ПОЛУЧИЛИ СПИСОК ДОСТУПНЫХ ТОЧЕК ДОСТУПА
        this.setState({NetList: JSON.parse(data.NetList)});
      } else if (data.DevsList) {                                   // ПОЛУЧИЛИ СПИСОК УСТРОЙСТВ В СЕТИ
        this.setState({DevsList: JSON.parse(data.DevsList)});
      } else if (data.UpdCheck) {                                   // ПОЛУЧИЛИ ОТВЕТ О НАЛИЧИИ ОБНОВЛЕНИЯ
        this.setState({UpdCheck: JSON.parse(data.UpdCheck)});
      } else if (data.updPrgMCU) {                                  // ПОЛУЧИЛИ ПРОГРЕСС ОБНОВЛЕНИЯ MCU
        this.setState({updPrgMCU: data.updPrgMCU});
      } else if (data.updPrgFFS) {                                  // ПОЛУЧИЛИ ПРОГРЕСС ОБНОВЛЕНИЯ FFS
        this.setState({updPrgFFS: data.updPrgFFS});
      } else if (data.updPrgESP) {                                  // ПОЛУЧИЛИ ПРОГРЕСС ОБНОВЛЕНИЯ ESP
        this.setState({updPrgESP: data.updPrgESP});
      } else if (data.infoMessage) {                                // ПОЛУЧИЛИ ИНФОРМАЦИОННОЕ СООБЩЕНИЕ
        this.setState({infoMessage: data.infoMessage});
      }
    };

    this.shouldReconnect = this.props.attempts;
    websocket.onclose = () => {
      if (this.shouldReconnect) {
        this.timeoutID = setTimeout(() => {
          this.setState({attempts: this.state.attempts - 1});
          this.setState({ws: new WebSocket("ws://" + location.hostname + "/ws", ["soi-tech"])});
          this.setupWebsocket();
        }, 100);
      }
      if (confirm("Communication Lost!\nUpdate this page?")) {
        window.location.reload();
      }
    };
  }

  // ОТПРАВКА ПАКЕТОВ В WEBSOCKET
  sendWS(event, d) {
    let data;
    switch (event) {
      case "ActualInfo":            // АКТУАЛЬНЫЕ ДАННЫЕ ОБ УСТРОЙСТВЕ
        data = `{"ActualInfo":[]}`;
        break;
      case "NetList":               // СПИСОК НАЙДЕННЫХ ТОЧЕК ДОСТУПА
        data = `{"NetList":[]}`;
        break;
      case "DevScan":               // ЗАПУСК СКАНИРОВАНИЯ УСТРОЙСТВ В СЕТИ
        data = `{"DevScan":[]}`;
        break;
      case "DevsList":              // СПИСОК НАЙДЕННЫХ УСТРОЙСТВ В СЕТИ
        data = `{"DevsList":[]}`;
        break;
      case "SetID":                 // УСТАНОВКА ИДЕНТИФИКАЦИОННЫХ ПАРАМЕТРОВ УСТРОЙСТВА (Цветовая зона, Имя устройства, Имя пользователи и пароль к Web интерфейсу)
        let {dc, dn, du, dp} = this.state.device;
        data = `{"SetID":["${dc}","${dn}","${du}","${dp}"]}`;
        break;
      case "SetSTA":                // УСТАНОВКА ПАРАМЕТРОВ РЕЖИМ STA (SSID, Пароль, если указаны статические параметры то еще IP адрес, Маска подсети, IP адрес шлюза)
        let {staSSID, staPSK, staIPMode, staIP, staMS, staGW} = this.state.device;
        data = `{"SetSTA":["${staSSID}","${staPSK}","${staIPMode}","${staIP}","${staMS}","${staGW}"]}`;
        break;
      case "SetRF":                 // УСТАНОВКА ПАРАМЕТРОВ РАДИО СЕТИ (Адрес устойства в радио сети, частотный канал)
        let {da, rch} = this.state.device;
        data = `{"SetRF":["${da}","${rch}"]}`;
        break;
      case "SetAP":                 // УСТАНОВКА ПАРАМЕТРОВ РЕЖИМ AP (Имя точки доступа, Пароль, IP адрес, Маска подсети, IP адрес шлюза)
        let {apSSID, apPSK, apIP, apMS, apGW} = this.state.device;
        data = `{"SetAP":["${apSSID}","${apPSK}","${apIP}","${apMS}","${apGW}"]}`;
        break;
      case "SetSC":                 // СОХРАНЕНИЕ СЦЕНАРИЕВ (JSON пакет)
        data = `{"SetSC":${d}}`;
        break;
      case "dv":                    // УСТАНОВКА ВИЗУАЛЬНОЙ ТЕМЫ (стили оформления Web интерфейса)
        let dv = this.state.dv;
        data = `{"DV":["${dv}"]}`;
        break;
      case "IOs":                   // ИМЕНОВАНИЕ I/O устройства (пользовательские названия, для теста)
        let {in1, in2} = this.state.device;
        data = `{"IOs":["${in1}","${in2}"]}`;
        break;
      case "Synch":                 // ПЕРЕДАЧА ПАРАМЕТРОВ АВТОРИЗАЦИИ (команда для рассылки SSID сети и пароля к ней по радио сети, чтоб не перенастраивать все устройства при смене роутера)
        data = `{"Synch":[]}`;
        break;
      case "UpdCheck":              // ПРОВЕРКА НАЛИЧИЯ ОБНОВЛЕНИЯ (булевый ответ)
        data = `{"UpdCheck":[]}`;
        break;
      case "UpdESPStart":           // ЗАПУСК ОБНОВЛЕНИЯ ESP (сначала обновляется FFS потом CORE)
        data = `{"UpdStart":["esp"]}`;
        break;
      case "UpdMCUStart":           // ЗАПУСК ОБНОВЛЕНИЯ MCU (BIN файл прошивки шлется в MCU с контролем потока)
        data = `{"UpdStart":["mcu"]}`;
        break;
      case "UpdStart":              // ЗАПУСК ОБНОВЛЕНИЯ УСТРОЙСТВА ПОЛНОСТЬЮ (отсутсвует в WEB интерфейсе, если это WiFi программатор)
        data = `{"UpdStart":["all"]}`;
        break;
      case "UpdESPStartID":         // ЗАПУСК ОБНОВЛЕНИЯ ESP по ID (сначала обновляется FFS потом CORE)
        let {id1, id2} = this.state.device;
        data = `{"UpdStart":["${id1}${id2}"]}`;
        break;
      case "UpdMCUClear":           // Удаление файлов прошивок из файловой системы
        data = `{"UpdClear":[]}`;
        break;
      case "Rst":                   // ПЕРЕЗАГРУЗКА УСТРОЙСТВА (сначала перезагружается MCU, потом ESP)
        data = `{"Rst":["all"]}`;
        break;
      case "DefSettings":                   // СБРОС НАСТРОЕК ПО УМОЛЧАНИЮ (настройки храняться в памяти программ)
        data = `{"DefSettings":[]}`;
        break;
      case "rs":                    // RangeSingle (для теста)
        data = `{"rs":${d}}`;
        break;
      case "rm":                    // RangeMulti (для теста)
        data = `{"rm":[${d}]}`;
        break;
      case "Settings":              // Настройки устройства (не WiFi модуля)
        data = `{"Settings":[]}`;
        break;
      default:                      // Прямая передача пакета из компонента и имеет вид {"mcu":["id","val"]} - пока не используется
        console.log("WS - Unknown Data")
    }
    let websocket = this.state.ws;
    websocket.send(data);
  }

  // ИНТЕРФЕЙС (язык)
  setLang(event) {
    return fetchJSON(`/lang?get=${event.target.value}&set=1`)
      .then(dictionary => {
        this.setState({dictionary, dl: event.target.value});
      })
      .catch(e => {
        console.error(e);
      });
  }

  // ИНТЕРФЕЙС (тема)
  setTheme(event) {
    let dv = event.target.value;
    this.setState({dv: dv});
    this.sendWS("dv");
  }

  componentWillMount() {
    window.addEventListener("hashchange", ::this.onHashChange);
    this.onHashChange();
    Promise.all([
      fetchJSON("/config")
        .then(device => {
          this.setState({device});
          this.setState({dv: device.dv});
          this.setState({dls: JSON.parse(device.dls).sort()});
          return fetchJSON(`/lang?get=${device.dl}&set=0`)
            .then(dictionary => this.setState({dictionary, dl: device.dl}))
            .catch(e => {
              console.error(e);
            });
        })
        .catch(e => {
          console.error(e);
        })
    ])
      .then(() => this.setState({loading: false}));
  }

  onHashChange() {
    let {hash} = window.location;
    switch (hash) {
      case "#network":
      case "#control":
      case "#scripts":
      case "#settings":
        break;
      default:
        hash = "";
        break;
    }
    this.setState({activeRoute: hash});
  }

  render(props, {activeRoute, device}) {
    return (
      <div id="wrapper">
        <Body dv={this.state.dv || null} loading={this.state.loading}/>
        {!this.state.loading &&
        <div>
          <a target="_blank" href="http://soi-tech.com">
            <div className="info" hidden>
              <p className="bold"><Message key="lowres"/></p>
            </div>
            <div className="logo"/>
          </a>
          <div className="main">
            <input id="lbch" type="checkbox" checked/>
            <label id="lCheck" for="lbch" className="lbcheck"/>
            <div className="lbar">
              {activeRoute === "" && <ManagerLeft/>}
              {activeRoute === "#network" && <NetworkLeft/>}
              {activeRoute === "#control" && <ControlLP/>}
              {activeRoute === "#scripts" && <ScriptsLP/>}
              {activeRoute === "#settings" && <SettingsLeft/>}
            </div>
            <input id="rbch" type="checkbox" checked/>
            <label id="rCheck" for="rbch" className="rbcheck"/>
            <div className="rbar">
              {activeRoute === "" && <ManagerRight/>}
              {activeRoute === "#network" && <NetworkRight/>}
              {activeRoute === "#control" && <ControlRP/>}
              {activeRoute === "#scripts" && <ScriptsRP/>}
              {activeRoute === "#settings" && <SettingsRight/>}
            </div>
            <h1 id="nameDF" className="header">{device.dm} || {device.dn}</h1>
            <div id="menu">
              <ul>
                <li>
                  <a href="#" className={activeRoute === "" ? "active" : ""}>
                    <div class="icon-container">
                      <div className="icon icon-manager"/>
                    </div>
                    <span className="mt"><Message key="menu.1"/></span>
                  </a>
                </li>
                <li>
                  <a href="#network" className={activeRoute === "#network" ? "active" : ""}>
                    <div class="icon-container">
                      <div className="icon icon-network"/>
                    </div>
                    <span className="mt"><Message key="menu.2"/></span>
                  </a>
                </li>
                <li>
                  <a href="#control" className={activeRoute === "#control" ? "active" : ""}>
                    <div class="icon-container">
                      <div className="icon icon-control"/>
                    </div>
                    <span className="mt"><Message key="menu.3"/></span>
                  </a>
                </li>
                <li>
                  <a href="#scripts" className={activeRoute === "#scripts" ? "active" : ""}>
                    <div class="icon-container">
                      <div className="icon icon-script"/>
                    </div>
                    <span className="mt"><Message key="menu.4"/></span>
                  </a>
                </li>
                <li>
                  <a href="#settings" className={activeRoute === "#settings" ? "active" : ""}>
                    <div class="icon-container">
                      <div className="icon icon-settings"/>
                    </div>
                    <span className="mt"><Message key="menu.5"/></span>
                  </a>
                </li>
              </ul>
            </div>
            <div className="cdl" style={`background-color: #${device.dc}`}/>
            <div id="content" className="content">
              {activeRoute === "" && <ManagerMain/>}
              {activeRoute === "#network" && <NetworkMain/>}
              {activeRoute === "#control" && <ControlPage/>}
              {activeRoute === "#scripts" && <ScriptsPage/>}
              {activeRoute === "#settings" && <SettingsMain/>}
            </div>
            <div className="footer">
              <Message key="footer.1"/>
              <a target="_blank" href="http://soi-tech.com">SOI Tech</a>
              <Message key="footer.2"/>
            </div>
          </div>
        </div>
        }
      </div>
    );
  }
}

export default App;
