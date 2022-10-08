import {h, Component} from "preact";
import {fetchJSON, translate} from "../util.jsx";
import Button from "./Button.jsx";
import Input from "./Input.jsx";
import Select from "./Select.jsx";
import Message from "./Message.jsx";
import Loading from "./Loading.jsx";

const EVENTS = 0;
const ACTIONS = 1;

const DeviceList = ({DevsList, selectedDevice, onChange}) => (
  <Select className="flex-elem fe1 select small"
          style={`border: 1px solid #${DevsList[selectedDevice.da].dc}`}
          onChange={onChange} value={selectedDevice.da}>
    {
      Object.keys(DevsList).map(key => {
        let {da, dm, dn} = DevsList[key];
        return <option value={da}>{`${dm}: ${dn}`}</option>;
      })
    }
  </Select>
);

const DeviceAPI = ({type, api, dictionary, selectedData, deletable, onCategoryChange, onSubcategoryChange, onAddButtonClick, onRemoveButtonClick, color}) => {
  const [selectedCategory, selectedSubcategory] = selectedData;
  return (
    <div className="flex-cont">
      <Select className="flex-elem fe4 select small" style={`border: 1px solid #${color};`}
              onChange={onCategoryChange}
              value={selectedCategory}>
        {api.map((_, i) =>
          <option value={i}>{translate(dictionary, `api.${type}.${i}.name`)}</option>
        )}
      </Select>
      <Select className="flex-elem fe5 select small" style={`border: 1px solid #${color}`}
              onChange={onSubcategoryChange}
              value={selectedSubcategory}>
        {new Array(api[selectedCategory]).fill(null).map((_, i) => {
          return <option
            value={i}>{translate(dictionary, `api.${type}.${selectedCategory}.options.${i}`)}</option>;
        })}
      </Select>
      <Button className="flex-elem fe6 button e2 gray small" onClick={deletable && onRemoveButtonClick}>-</Button>
      <Button className="flex-elem fe7 button e2 blue small" onClick={onAddButtonClick}>+</Button>
      <Button className="flex-elem fe8 button e2 empty small"/>
    </div>
  );
};

const loadDeviceData = ip => {
  return (
    fetchJSON(`//${ip}/config`)
      .then(function (json) {
        const device = {ip: ip, status: "loading", ...json};
        return fetchJSON(`//${ip}/api`)
          .then(api => {
            device.api = api;
            return fetchJSON(`//${ip}/lang?get=${json.dl}&set=0`)
              .then(dl => {
                device.dictionary = dl;
                device.status = "ok";
                return device;
              });
          });
      })
      .catch((e) => ( {ip: ip, status: "error"}))
  );
};

class Scripts extends Component {
  constructor() {
    super();
    this.setState({
      scripts: [],
      DevsList: {},
      loading: true
    });
  }

  componentWillMount() {
    this.setState({loading: true});
    this.context.DevsList.map((item, index) => {
      if (index === 0) {
        this.setState({defaultDevice: item.da});
      }
      loadDeviceData(item.ip)
        .then(data => {
          let {DevsList} = this.state;
          DevsList[item.da] = data;
          this.setState({DevsList});
          return fetchJSON("/scr");
        })
        .then(data => this.setState({
          scripts: data.map(script => {
            script.collapsed = true;
            return script;
          })
        }))
        .catch(e => {
          this.setState({scripts: []});
          console.error(e);
        })
        .then(() => this.setState({loading: false}));
    });
  }

  // Удалить текущий сценарий
  removeScript(index) {
    const {scripts} = {...this.state};
    scripts.splice(index, 1);
    this.setState({scripts});
  }

  // Удалить все сценарии (передаем пустой массив)
  removeScripts() {
    this.setState({scripts: []});
    this.saveScripts();
  }

  // Создать сценарий
  createScript() {
    const {scripts, DevsList, defaultDevice} = {...this.state};
    const localDevice = DevsList[defaultDevice];
    scripts.push({
      collapsed: false,
      NM: `-= ${scripts.length} =-`,
      EN: false,
      AP: 0,
      EV: [{da: localDevice.da, pr: [[0, 0]]}],
      AC: [{da: localDevice.da, pr: [[0, 0]]}]
    });
    this.setState({scripts});
  }

  // Сохранение сценариев
  saveScripts() {
    this.setState({loading: true});
    const {scripts} = this.state;
    const scData = JSON.stringify(scripts.map(({NM, EN, AP, EV, AC}) => ({NM, EN, AP, EV, AC})));
    this.context.sendWS("SetSC", scData);
    this.setState({loading: false});
  }

  onNameInput(index, event) {
    const {scripts} = {...this.state};
    scripts[index].NM = event.target.value.substring(0, 8);
    this.setState({scripts});
  }

  onCustomParamInput(index, event) {
    const {scripts} = {...this.state};
    scripts[index].AP = event.target.value.replace(/[^0-9]/g, "").substring(0, 4);
    this.setState({scripts});
  }

  toggleActiveState(index) {
    const {scripts} = {...this.state};
    scripts[index].EN = +!scripts[index].EN;
    this.setState({scripts});
  }

  addDevice(category, scriptIndex, after) {
    const {scripts, DevsList, defaultDevice} = {...this.state};
    const device = DevsList[defaultDevice];
    const {da} = device;
    let target;
    let data;
    if (category === EVENTS) {
      target = scripts[scriptIndex].EV;
      data = {da, pr: [[0, 0]]};
    } else if (category === ACTIONS) {
      target = scripts[scriptIndex].AC;
      data = {da, pr: [[0, 0]]};
    }
    if (target) {
      target.splice(++after, 0, data);
      this.setState({scripts});
    } else
      console.error("Error Adding Device");
  }

  removeDevice(category, scriptIndex, selectedDeviceIndex) {
    const {scripts} = {...this.state};
    let target;
    if (category === EVENTS)
      target = scripts[scriptIndex].EV;
    else if (category === ACTIONS)
      target = scripts[scriptIndex].AC;
    if (target) {
      target.splice(selectedDeviceIndex, 1);
      this.setState({scripts});
    } else
      console.error("Error Removing Device");
  }

  onDeviceChange(category, scriptIndex, selectedDeviceIndex, e) {
    const {scripts, DevsList} = {...this.state};
    const device = DevsList[e.target.value];
    let target = category === EVENTS ? scripts[scriptIndex].EV : scripts[scriptIndex].AC;
    let data = [0, 0];
    if (target) {
      target[selectedDeviceIndex] = {
        da: device.da,
        pr: [data]
      };
      this.setState({scripts});
    } else
      console.error("Error Changing Device");
  }

  onCategoryChange(category, scriptIndex, selectedDeviceIndex, selectedIndex, e) {
    const {scripts} = {...this.state};
    let target = category === EVENTS ?
      scripts[scriptIndex].EV[selectedDeviceIndex].pr :
      scripts[scriptIndex].AC[selectedDeviceIndex].pr;
    let data = [+e.target.value, 0];
    if (target) {
      target[selectedIndex] = data;
      this.setState({scripts});
    } else
      console.error("Error Changing Category");
  }

  onSubcategoryChange(category, scriptIndex, selectedDeviceIndex, selectedIndex, e) {
    const {scripts} = {...this.state};
    let target = category === EVENTS ?
      scripts[scriptIndex].EV[selectedDeviceIndex].pr :
      scripts[scriptIndex].AC[selectedDeviceIndex].pr;
    if (target) {
      target[selectedIndex][1] = +e.target.value;
      this.setState({scripts});
    } else
      console.error("Error Changing Subcategory");
  }

  onAddButtonClick(category, scriptIndex, selectedDeviceIndex, after) {
    const {scripts} = {...this.state};
    let target = category === EVENTS ?
      scripts[scriptIndex].EV[selectedDeviceIndex].pr :
      scripts[scriptIndex].AC[selectedDeviceIndex].pr;
    let data = [0, 0];
    if (target) {
      target.splice(++after, 0, data);
      this.setState({scripts});
    } else
      console.error("Error Adding");
  }

  onRemoveButtonClick(category, scriptIndex, selectedDeviceIndex, selectedIndex) {
    const {scripts} = {...this.state};
    let target = category === EVENTS ?
      scripts[scriptIndex].EV[selectedDeviceIndex].pr :
      scripts[scriptIndex].AC[selectedDeviceIndex].pr;
    if (target) {
      target.splice(selectedIndex, 1);
      this.setState({scripts});
    } else
      console.error("Error Removing");
  }

  toggleCollapseScript(scriptIndex, event) {
    const {scripts} = {...this.state};
    scripts[scriptIndex].collapsed = !event.target.checked;
    this.setState({scripts});
  }

  render() {
    const {DevsList, scripts} = {...this.state};
    return this.state.loading ? <Loading/> : (
      <div>
        {
          scripts && scripts.map((script, scriptIndex) => (
            <div className="sb">
              <div className="sc-main">

                <input type="checkbox" className="switch-hor" id={"switch-hor" + scriptIndex}
                       onChange={this.toggleActiveState.bind(this, scriptIndex)}
                       checked={script.EN}/>
                <label for={"switch-hor" + scriptIndex} className="sc-switch"/>

                <Input className="sc-name w75" type="text" value={script.NM} maxlength="8"
                       onInput={this.onNameInput.bind(this, scriptIndex)}>{script.NM}</Input>

                <div className="icon-delete"><Button className="delete"
                       onClick={this.removeScript.bind(this, scriptIndex)}/></div>

                <input type="checkbox" id={"sc-collapse" + scriptIndex} className="checker"
                       onChange={this.toggleCollapseScript.bind(this, scriptIndex)}
                       checked={script.collapsed !== true}/>
                <label for={"sc-collapse" + scriptIndex} className="toggle"/>

                <div className="spl">
                  <h3 className="bold tsc"><Message key="scripts.event"/></h3>
                  <div>
                    {
                      script.EV.map((selectedDevice, selectedDeviceIndex) => (
                        (DevsList.hasOwnProperty(selectedDevice.da) && (DevsList[selectedDevice.da].status === "ok")) ?
                          (<div className="sc-device">
                            <div className="flex-cont">
                              <DeviceList DevsList={DevsList}
                                          selectedDevice={selectedDevice}
                                          onChange={this.onDeviceChange.bind(this, EVENTS, scriptIndex, selectedDeviceIndex)}/>
                              <Button className="flex-elem fe2 button e2 gray small"
                                      onClick={(script.EV.length > 1) && this.removeDevice.bind(this, EVENTS, scriptIndex, selectedDeviceIndex)}>-</Button>
                              <Button className="flex-elem fe3 button e2 blue small"
                                      onClick={this.addDevice.bind(this, EVENTS, scriptIndex, selectedDeviceIndex)}>+</Button>
                            </div>
                            {
                              selectedDevice.pr.map((selected, index) => {
                                const device = DevsList[selectedDevice.da];
                                return (
                                  <DeviceAPI
                                    type={EVENTS}
                                    color={device.dc}
                                    api={device.api[EVENTS]}
                                    deletable={selectedDevice.pr.length > 1}
                                    dictionary={device.dictionary}
                                    selectedData={selected}
                                    onCategoryChange={this.onCategoryChange.bind(this, EVENTS, scriptIndex, selectedDeviceIndex, index)}
                                    onSubcategoryChange={this.onSubcategoryChange.bind(this, EVENTS, scriptIndex, selectedDeviceIndex, index)}
                                    onAddButtonClick={this.onAddButtonClick.bind(this, EVENTS, scriptIndex, selectedDeviceIndex, index)}
                                    onRemoveButtonClick={this.onRemoveButtonClick.bind(this, EVENTS, scriptIndex, selectedDeviceIndex, index)}
                                  />
                                );
                              })
                            }

                          </div>) : <Loading/>
                      ))
                    }
                  </div>
                  <br/>
                  <h3 className="bold tsc"><Message key="scripts.action"/></h3>
                  <div>
                    {
                      script.AC.map((selectedDevice, selectedDeviceIndex) => (
                        (DevsList.hasOwnProperty(selectedDevice.da) && (DevsList[selectedDevice.da].status === "ok")) ?
                          (<div className="sc-device">
                            <div className="flex-cont">
                              <DeviceList DevsList={DevsList}
                                          selectedDevice={selectedDevice}
                                          onChange={this.onDeviceChange.bind(this, ACTIONS, scriptIndex, selectedDeviceIndex)}/>
                              <Button className="flex-elem fe2 button e2 gray small"
                                      onClick={(script.AC.length > 1) && this.removeDevice.bind(this, ACTIONS, scriptIndex, selectedDeviceIndex)}>-</Button>
                              <Button className="flex-elem fe3 button e2 blue small"
                                      onClick={this.addDevice.bind(this, ACTIONS, scriptIndex, selectedDeviceIndex)}>+</Button>
                            </div>
                            {
                              selectedDevice.pr.map((selected, index) => {
                                const device = DevsList[selectedDevice.da];
                                return (
                                  <DeviceAPI
                                    type={ACTIONS}
                                    color={device.dc}
                                    api={device.api[ACTIONS]}
                                    dictionary={device.dictionary}
                                    deletable={selectedDevice.pr.length > 1}
                                    selectedData={selected}
                                    onCategoryChange={this.onCategoryChange.bind(this, ACTIONS, scriptIndex, selectedDeviceIndex, index)}
                                    onSubcategoryChange={this.onSubcategoryChange.bind(this, ACTIONS, scriptIndex, selectedDeviceIndex, index)}
                                    onAddButtonClick={this.onAddButtonClick.bind(this, ACTIONS, scriptIndex, selectedDeviceIndex, index)}
                                    onRemoveButtonClick={this.onRemoveButtonClick.bind(this, ACTIONS, scriptIndex, selectedDeviceIndex, index)}
                                  />
                                );
                              })
                            }
                          </div>) : <Loading/>
                      ))
                    }
                  </div>
                  {/* Дополнительный параметр */}
                  <input type="checkbox" id={"sc-param" + scriptIndex} className="sc-checker"/>
                  <label for={"sc-param" + scriptIndex} className="sc-toggle"/>
                  <div className="sc-cont">
                    <p className="bold"><Message key="scripts.advpar"/></p>
                    <div>
                      <Input className="input w75 large" type="text"
                             value={script.AP}
                             onInput={this.onCustomParamInput.bind(this, scriptIndex)}/>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          ))
        }
        <div><Button className="button w70 blue large" onClick={::this.createScript}><Message
          key="scripts.create"/></Button></div>

          <div><Button className="button w70 blue large" onClick={::this.saveScripts}><Message
              key="scripts.save"/></Button></div>&nbsp;&nbsp;

        <div><Button className="button w70 red large" onClick={::this.removeScripts}><Message
          key="scripts.clear"/></Button></div>
      </div>
    );
  }
}

export default Scripts;
