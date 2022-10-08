// Страница "СЦЕНАРИИ" (центр)

import {h, Component} from "preact";
import Scripts from "../components/Scripts.jsx";
import Message from "../components/Message.jsx";


export default class ScriptsPage extends Component {
  render() {
    return (
      <div>
        <h3 className="tPan"><Message key="scripts.title"/></h3>
        <div>
          <Scripts/>
        </div>
      </div>
    );
  }
}
