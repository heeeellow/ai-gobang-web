import { createApp } from 'vue'
import App from './App.vue'
import router from './router' // 路由稍后配置
import './assets/main.css'

const app = createApp(App)
app.use(router)
app.mount('#app')
